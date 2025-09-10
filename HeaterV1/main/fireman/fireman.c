// Heater (Fireman) control with PID loop and periodic temperature polling.
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "mailman/i2c_task.h"
#include "pinout.h"
#include "fireman.h"

#define BOARD_MAX_TEMPERATURE_C            60.0f  /* °C – shut off above this */

#define SAMPLING_PERIOD_MILLISECONDS       500     /* 2 Hz control loop       */

#define PROPORTIONAL_GAIN                  2.0f
#define INTEGRAL_GAIN                      0.03f  /* 1/s */
#define DERIVATIVE_GAIN                    4.0f   /* s   */
#define DERIVATIVE_CLAMP_C_PER_SECOND      5.0f
#define MAX_DUTY               ((1 << 13) - 1)

ledc_channel_config_t heater_channel_1 = {
    .channel    = LEDC_CHANNEL_2,
    .duty       = 0,
    .gpio_num   = HEATER_CHANNEL_1,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .hpoint     = 0,
    .timer_sel  = LEDC_TIMER_3,
    .flags.output_invert = 0
};

ledc_channel_config_t heater_channel_2 = {
    .channel    = LEDC_CHANNEL_3,
    .duty       = 0,
    .gpio_num   = HEATER_CHANNEL_2,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .hpoint     = 0,
    .timer_sel  = LEDC_TIMER_3,
    .flags.output_invert = 0
};

typedef struct {
    float proportionalGain;      /* Kp */
    float integralGain;          /* Ki */
    float derivativeGain;        /* Kd */

    float integralAccumulator;   /* ∑ error · dt */
    float previousError;         /* error[k‑1]   */

    float outputMinimumPercent;  /* e.g. 0   */
    float outputMaximumPercent;  /* e.g. 100 */
} PID_Controller;

typedef struct {
    double ch1;
    double ch2;
} fireman_sample_t; // what we'd send to jumbotron

static volatile int setpoint1_c  = 0; /* Desired temperature ch1 (modified by other tasks) */
static volatile int setpoint2_c  = 0; /* Desired temperature ch2 (modified by other tasks) */
static PID_Controller pid1;
static PID_Controller pid2;
static QueueHandle_t g_i2c_queue = NULL;
static QueueHandle_t g_jumbo_queue = NULL; // mock display queue
static volatile bool heater1_enabled = false;  /* Cross-task flag */
static volatile bool heater2_enabled = false;  /* Cross-task flag */
/* PD management: track current fixed voltage (5,9,15,20). PoR default is 5V */
static int current_pd_voltage = 5;
static bool prev_any_enabled = false;        /* Detect transitions to/from active heating */

static inline float clamp_float(float value, float minimum, float maximum)
{
    if (value > maximum) return maximum;
    if (value < minimum) return minimum;
    return value;
}

void PIDController_Init(PID_Controller *controller, float proportionalGain, float integralGain, float derivativeGain,  float outputMinimumPercent,  float outputMaximumPercent)
{
    controller->proportionalGain    = proportionalGain;
    controller->integralGain        = integralGain;
    controller->derivativeGain      = derivativeGain;

    controller->integralAccumulator = 0.0f;
    controller->previousError       = 0.0f;

    controller->outputMinimumPercent = outputMinimumPercent;
    controller->outputMaximumPercent = outputMaximumPercent;
}

float PIDController_Compute(PID_Controller *controller, float setPoint, float measuredValue, float deltaTimeSeconds)
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

bool fireman_setup(QueueHandle_t i2c_queue, QueueHandle_t jumbotron_queue)
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
    g_jumbo_queue = jumbotron_queue; // may be NULL (mock)
    xTaskCreate(fireman_task, "fireman", 4096, NULL, 8, NULL);

    return true;
}

void fireman_set_heater1_enabled(bool en) { heater1_enabled = en; }
void fireman_set_heater2_enabled(bool en) { heater2_enabled = en; }
void fireman_set_setpoint1(int setpoint_c) { setpoint1_c = setpoint_c; }
void fireman_set_setpoint2(int setpoint_c) { setpoint2_c = setpoint_c; }
void fireman_set_setpoints(int sp1_c, int sp2_c) { setpoint1_c = sp1_c; setpoint2_c = sp2_c; }

static esp_err_t request_adc(adc_result_t *out) {
    i2c_msg_t msg = {0};
    msg.type = I2C_MSG_ADC_READ_BOTH;
    QueueHandle_t resp = xQueueCreate(1, sizeof(adc_result_t));
    if (!resp) return ESP_ERR_NO_MEM;
    msg.response_queue = resp;
    if (xQueueSend(g_i2c_queue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) { vQueueDelete(resp); return ESP_ERR_TIMEOUT; }
    if (xQueueReceive(resp, out, pdMS_TO_TICKS(200)) != pdTRUE) { vQueueDelete(resp); return ESP_ERR_TIMEOUT; }
    vQueueDelete(resp);
    return ESP_OK;
}

/* Helper: request a specific fixed voltage (5,9,15,20) and wait for boolean success */
static bool send_pd_voltage(uint8_t voltage) {
    if (!g_i2c_queue) return false;
    i2c_msg_t msg = {0};
    msg.type = I2C_MSG_PD_SET_PDO;
    msg.data.pd_set.set_voltage = voltage; /* actual voltage value */
    QueueHandle_t resp = xQueueCreate(1, sizeof(bool));
    if (!resp) return false;
    msg.response_queue = resp;
    bool ok = false;
    if (xQueueSend(g_i2c_queue, &msg, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (xQueueReceive(resp, &ok, pdMS_TO_TICKS(200)) != pdTRUE) ok = false;
    }
    vQueueDelete(resp);
    return ok;
}

/* Adjust PD voltage according to heater usage.
   Policy: when any heater enabled -> request 20V, fallback 15V.
            When no heaters enabled -> request 5V. */
static void manage_pd_voltage(void) {
    bool any_enabled = heater1_enabled || heater2_enabled;
    if (any_enabled && !prev_any_enabled) {
        if (send_pd_voltage(20)) {
            current_pd_voltage = 20;
        } else if (send_pd_voltage(15)) {
            current_pd_voltage = 15;
        } else {
            // Could not secure higher voltage. Disable heaters for safety.
            heater1_enabled = false;
            heater2_enabled = false;
            current_pd_voltage = 5; // remains at default 5V
            if (g_jumbo_queue) {
                fireman_sample_t err_sample = { .ch1 = NAN, .ch2 = NAN }; // NANs indicate PD failure
                (void)xQueueSend(g_jumbo_queue, &err_sample, 0);
            }
        }
    } else if (!any_enabled && prev_any_enabled) {
        if (current_pd_voltage != 5) {
            if (send_pd_voltage(5)) current_pd_voltage = 5; /* ignore failure otherwise */
        }
    }
    prev_any_enabled = any_enabled;
}

static void fireman_task(void *arg) {
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        adc_result_t adc = {0};
        if (g_i2c_queue && request_adc(&adc) == ESP_OK) {
            /* Handle PD voltage transitions based on heater enable flags */
            manage_pd_voltage();
            // Send to jumbotron (mock)
            if (g_jumbo_queue) {
                fireman_sample_t sample = { .ch1 = adc.chan1, .ch2 = adc.chan2 };
                (void)xQueueSend(g_jumbo_queue, &sample, 0);
            }

            float dt_s = (float)SAMPLING_PERIOD_MILLISECONDS / 1000.0f;
            // Channel 1 control
            if (heater1_enabled && setpoint1_c > 0) {
                float t1 = (float)adc.chan1;
                float duty1 = PIDController_Compute(&pid1, (float)setpoint1_c, t1, dt_s);
                uint32_t duty_val1 = (uint32_t)(MAX_DUTY * duty1 / 100.0f);
                ledc_set_duty(heater_channel_1.speed_mode, heater_channel_1.channel, duty_val1);
                ledc_update_duty(heater_channel_1.speed_mode, heater_channel_1.channel);
            } else {
                ledc_set_duty(heater_channel_1.speed_mode, heater_channel_1.channel, 0);
                ledc_update_duty(heater_channel_1.speed_mode, heater_channel_1.channel);
                pid1.integralAccumulator = 0.0f; pid1.previousError = 0.0f;
            }
            // Channel 2 control
            if (heater2_enabled && setpoint2_c > 0) {
                float t2 = (float)adc.chan2;
                float duty2 = PIDController_Compute(&pid2, (float)setpoint2_c, t2, dt_s);
                uint32_t duty_val2 = (uint32_t)(MAX_DUTY * duty2 / 100.0f);
                ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, duty_val2);
                ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);
            } else {
                ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, 0);
                ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);
                pid2.integralAccumulator = 0.0f; pid2.previousError = 0.0f;
            }
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SAMPLING_PERIOD_MILLISECONDS));
    }
}

// Legacy interactive PID test removed in favor of autonomous task.
