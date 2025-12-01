// Heater (Fireman) control with PID loop and periodic temperature polling.
#include "fireman.h"  // public API (must come first for struct declarations)
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "pwm_alloc.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "mailman/mailman.h"
#include "pinout.h"
#include "wiseman/wiseman.h"
#include "HeaterGUI_gen.h"

#define BOARD_MAX_TEMPERATURE_C            60.0f  /* °C – shut off above this */
#define CONTROL_PERIOD_MILLISECONDS_ACTIVE 500     /* 500 ms when heating active */
#define CONTROL_PERIOD_MILLISECONDS_IDLE   1500     /* 1.5 s when idle (both heaters disabled) */
#define PROPORTIONAL_GAIN                  2.0f
#define INTEGRAL_GAIN                      0.03f  /* 1/s */
#define DERIVATIVE_GAIN                    4.0f   /* s   */
#define DERIVATIVE_CLAMP_C_PER_SECOND      5.0f
#define MAX_DUTY                           ((1 << 13) - 1)

static const ledc_channel_config_t heater_channel_1 = {
    .channel    = PWM_HEATER_CH1,
    .duty       = 0,
    .gpio_num   = HEATER_CHANNEL_1,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .hpoint     = 0,
    .timer_sel  = PWM_HEATER_TIMER,
    .flags.output_invert = 0
};

static const ledc_channel_config_t heater_channel_2 = {
    .channel    = PWM_HEATER_CH2,
    .duty       = 0,
    .gpio_num   = HEATER_CHANNEL_2,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .hpoint     = 0,
    .timer_sel  = PWM_HEATER_TIMER,
    .flags.output_invert = 0
};

typedef struct {
    float proportionalGain;      /* Kp */
    float integralGain;          /* Ki */
    float derivativeGain;        /* Kd */
    float integralAccumulator;   /* ∑ error · dt */
    float previousError;         /* error[k‑1]   */
    float outputMinimumPercent;  /* e.g. 0 */
    float outputMaximumPercent;  /* e.g. 100 */
} PID_Controller;

// Types defined in header (fireman_sample_t, fireman_pd_caps_t)

static volatile int setpoint1_c  = 0; /* Desired temperature ch1 (modified by other tasks) */
static volatile int setpoint2_c  = 0; /* Desired temperature ch2 (modified by other tasks) */
static PID_Controller pid1;
static PID_Controller pid2;
static QueueHandle_t g_i2c_queue = NULL;
static QueueHandle_t g_jumbo_queue = NULL; // single-slot display queue (xQueueOverwrite)
static QueueHandle_t s_adc_resp_queue = NULL; // persistent response queue (avoid alloc per loop)
static volatile bool heater1_enabled = false;  /* Cross-task flag */
static volatile bool heater2_enabled = false;  /* Cross-task flag */

static void fireman_task(void *arg);

static inline float clamp_float(float value, float minimum, float maximum)
{
    if (value > maximum) return maximum;
    if (value < minimum) return minimum;
    return value;
}

static void PIDController_Init(PID_Controller *controller, float proportionalGain, float integralGain, float derivativeGain,  float outputMinimumPercent,  float outputMaximumPercent)
{
    controller->proportionalGain    = proportionalGain;
    controller->integralGain        = integralGain;
    controller->derivativeGain      = derivativeGain;

    controller->integralAccumulator = 0.0f;
    controller->previousError       = 0.0f;

    controller->outputMinimumPercent = outputMinimumPercent;
    controller->outputMaximumPercent = outputMaximumPercent;
}

static float PIDController_Compute(PID_Controller *controller, float setPoint, float measuredValue, float deltaTimeSeconds)
{
    /* 1. Instantaneous error */
    float error = setPoint - measuredValue;

    /* 2. Proportional component */
    float proportionalOutput = controller->proportionalGain * error;

    /* 3. Integral term with enhanced anti‑wind‑up */
    float tentativeIntegral = controller->integralAccumulator + (error * deltaTimeSeconds);
    float integralCeiling   = controller->outputMaximumPercent / controller->integralGain;
    float integralFloor     = controller->outputMinimumPercent / controller->integralGain;

    controller->integralAccumulator = clamp_float(tentativeIntegral, integralFloor, integralCeiling);
    float integralOutput = controller->integralGain * controller->integralAccumulator;

    /* 4. Derivative component with noise clamp */
    float derivative = (error - controller->previousError) / deltaTimeSeconds;
    derivative = clamp_float(derivative,  -DERIVATIVE_CLAMP_C_PER_SECOND, DERIVATIVE_CLAMP_C_PER_SECOND);
    float derivativeOutput = controller->derivativeGain * derivative;

    controller->previousError = error;

    /* 5. Combine components */
    float rawOutputPercent = proportionalOutput + integralOutput + derivativeOutput;

    /* 6. Apply output limits and unwind integral if clamped */
    if (rawOutputPercent > controller->outputMaximumPercent) {
        rawOutputPercent = controller->outputMaximumPercent;
        controller->integralAccumulator -= error * deltaTimeSeconds;
    } else if (rawOutputPercent < controller->outputMinimumPercent) {
        rawOutputPercent = controller->outputMinimumPercent;
        controller->integralAccumulator -= error * deltaTimeSeconds;
    }

    return rawOutputPercent;
}

bool fireman_init(QueueHandle_t i2c_queue, QueueHandle_t jumbotron_queue)
{
    ledc_timer_config_t heater_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 1000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_3,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&heater_timer);
    ledc_channel_config(&heater_channel_1);
    ledc_channel_config(&heater_channel_2);
    ledc_set_duty(heater_channel_1.speed_mode, heater_channel_1.channel, 0);
    ledc_update_duty(heater_channel_1.speed_mode, heater_channel_1.channel);
    ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, 0);
    ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);

    PIDController_Init(&pid1,
                       PROPORTIONAL_GAIN,
                       INTEGRAL_GAIN,
                       DERIVATIVE_GAIN,
                       0.0f,
                       100.0f);

    PIDController_Init(&pid2,
                       PROPORTIONAL_GAIN,
                       INTEGRAL_GAIN,
                       DERIVATIVE_GAIN,
                       0.0f,
                       100.0f);

    g_i2c_queue = i2c_queue;
    g_jumbo_queue = jumbotron_queue; // may be NULL

    // Initialize Fireman state from persisted settings
    const wiseman_settings_t* s = wiseman_get();
    // Heaters must be disabled on every PoR regardless of persisted state
    heater1_enabled = false;
    heater2_enabled = false;
    setpoint1_c = s->setpoint1_c;
    setpoint2_c = s->setpoint2_c;

    xTaskCreate(fireman_task, "fireman", 4096, NULL, 8, NULL);

    return true;
}

void fireman_set_heater1_enabled(bool en) { 
    heater1_enabled = en;
    // Update UI subject: heaterRunning is 1 if ANY heater is enabled
    lv_subject_set_int(&heaterRunning, (heater1_enabled || heater2_enabled) ? 1 : 0);
}

void fireman_set_heater2_enabled(bool en) { 
    heater2_enabled = en;
    // Update UI subject: heaterRunning is 1 if ANY heater is enabled
    lv_subject_set_int(&heaterRunning, (heater1_enabled || heater2_enabled) ? 1 : 0);
}

void fireman_set_setpoint1(int setpoint_c) {
     setpoint1_c = setpoint_c;
}

void fireman_set_setpoint2(int setpoint_c) {
    setpoint2_c = setpoint_c;
}

void fireman_set_setpoints(int sp1_c, int sp2_c) {
    setpoint1_c = sp1_c; 
    setpoint2_c = sp2_c;
}

static esp_err_t request_adc(adc_result_t *out) {
    if (!s_adc_resp_queue) {
        s_adc_resp_queue = xQueueCreate(1, sizeof(adc_result_t));
        if (!s_adc_resp_queue) return ESP_ERR_NO_MEM;
    }
    i2c_msg_t msg = {0};
    msg.type = I2C_MSG_ADC_READ_BOTH;
    msg.response_queue = s_adc_resp_queue;
    if (xQueueSend(g_i2c_queue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) return ESP_ERR_TIMEOUT;
    if (xQueueReceive(s_adc_resp_queue, out, pdMS_TO_TICKS(200)) != pdTRUE) return ESP_ERR_TIMEOUT;
    return ESP_OK;
}





static void fireman_task(void *arg) {
    TickType_t last_wake = xTaskGetTickCount();
    float last_sent_ch1_q = NAN;
    float last_sent_ch2_q = NAN;
    const uint32_t FORCE_REFRESH_LOOPS = 20; // heartbeat (approx 10s active)
    uint32_t loops_since_send = 0;
    uint32_t current_period_ms = CONTROL_PERIOD_MILLISECONDS_IDLE;
    uint32_t last_duty1 = UINT32_MAX;
    uint32_t last_duty2 = UINT32_MAX;
    // Accumulate heater "on" runtime in milliseconds (any channel enabled counts). We convert to minutes lazily.
    uint64_t accum_on_ms = 0;
    uint32_t last_reported_total_min = wiseman_get_op_time_minutes();
    while (1) {
        bool active = (heater1_enabled && setpoint1_c > 0) || (heater2_enabled && setpoint2_c > 0);
        uint32_t desired = active ? CONTROL_PERIOD_MILLISECONDS_ACTIVE : CONTROL_PERIOD_MILLISECONDS_IDLE;
        if (desired != current_period_ms) current_period_ms = desired;
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(current_period_ms));

        adc_result_t adc = {0};
        bool adc_ok = (g_i2c_queue && request_adc(&adc) == ESP_OK);

        if (adc_ok) {
            float dt_s = (float)current_period_ms / 1000.0f;

            // Over-temperature fail-safe: hard cutoff
            if (adc.chan1 > BOARD_MAX_TEMPERATURE_C || adc.chan2 > BOARD_MAX_TEMPERATURE_C) {
                heater1_enabled = false; heater2_enabled = false;
                pid1.integralAccumulator = pid1.previousError = 0.0f;
                pid2.integralAccumulator = pid2.previousError = 0.0f;
            }

            // Heater 1
            if (heater1_enabled && setpoint1_c > 0) {
                float duty1 = PIDController_Compute(&pid1, (float)setpoint1_c, (float)adc.chan1, dt_s);
                uint32_t duty_val1 = (uint32_t)(MAX_DUTY * duty1 / 100.0f);
                if (duty_val1 != last_duty1) {
                    ledc_set_duty(heater_channel_1.speed_mode, heater_channel_1.channel, duty_val1);
                    ledc_update_duty(heater_channel_1.speed_mode, heater_channel_1.channel);
                    last_duty1 = duty_val1;
                }
            } else {
                if (last_duty1 != 0) {
                    ledc_set_duty(heater_channel_1.speed_mode, heater_channel_1.channel, 0);
                    ledc_update_duty(heater_channel_1.speed_mode, heater_channel_1.channel);
                    last_duty1 = 0;
                }
                pid1.integralAccumulator = 0.0f; pid1.previousError = 0.0f;
            }

            // Heater 2
            if (heater2_enabled && setpoint2_c > 0) {
                float duty2 = PIDController_Compute(&pid2, (float)setpoint2_c, (float)adc.chan2, dt_s);
                uint32_t duty_val2 = (uint32_t)(MAX_DUTY * duty2 / 100.0f);
                if (duty_val2 != last_duty2) {
                    ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, duty_val2);
                    ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);
                    last_duty2 = duty_val2;
                }
            } else {
                if (last_duty2 != 0) {
                    ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, 0);
                    ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);
                    last_duty2 = 0;
                }
                pid2.integralAccumulator = 0.0f; pid2.previousError = 0.0f;
            }

            // Accumulate runtime if any heater is enabled (regardless of actual duty cycle; counts enabled time)
            if (active) {
                accum_on_ms += current_period_ms;
                if (accum_on_ms >= 60000ULL) { // at least 1 minute accumulated locally
                    uint32_t add_min = (uint32_t)(accum_on_ms / 60000ULL);
                    accum_on_ms -= (uint64_t)add_min * 60000ULL;
                    wiseman_add_op_time_minutes(add_min);
                    uint32_t new_total = wiseman_get_op_time_minutes();
                    last_reported_total_min = new_total; // director will poll and push subject later
                }
            }

            // After control update, send sample only if there is a meaningful change or heartbeat interval.
            if (g_jumbo_queue) {
                // Quantize to one decimal place (round to nearest 0.1)
                float q_ch1 = roundf(adc.chan1 * 10.0f) / 10.0f;
                float q_ch2 = roundf(adc.chan2 * 10.0f) / 10.0f;
                bool ch1_changed = isnan(last_sent_ch1_q) || q_ch1 != last_sent_ch1_q;
                bool ch2_changed = isnan(last_sent_ch2_q) || q_ch2 != last_sent_ch2_q;
                bool force_refresh = loops_since_send >= FORCE_REFRESH_LOOPS;
                if (ch1_changed || ch2_changed || force_refresh) {
                    fireman_sample_t sample = { .ch1 = adc.chan1, .ch2 = adc.chan2 };
                    (void)xQueueOverwrite(g_jumbo_queue, &sample);
                    last_sent_ch1_q = q_ch1;
                    last_sent_ch2_q = q_ch2;
                    loops_since_send = 0;
                } else {
                    loops_since_send++;
                }
            }
        } else {
            // ADC failed: force heaters off
            ledc_set_duty(heater_channel_1.speed_mode, heater_channel_1.channel, 0);
            ledc_update_duty(heater_channel_1.speed_mode, heater_channel_1.channel);
            ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, 0);
            ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);
            if (g_jumbo_queue) {
                fireman_sample_t err_sample = { .ch1 = NAN, .ch2 = NAN };
                (void)xQueueOverwrite(g_jumbo_queue, &err_sample);
                last_sent_ch1_q = NAN; // Reset so next valid reading transmits immediately
                last_sent_ch2_q = NAN;
                loops_since_send = 0;
            }
        }
    }
}

