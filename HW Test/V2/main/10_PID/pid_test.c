#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "ADS7142.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include <unistd.h>


#define BOARD_MAX_TEMPERATURE_C            60.0f  /* °C – shut off above this */
#define VALUE_PER_BIT                     (0.00005035477) 

#define SAMPLING_PERIOD_MILLISECONDS       500     /* 2 Hz control loop       */

#define PROPORTIONAL_GAIN                  2.0f
#define INTEGRAL_GAIN                      0.03f  /* 1/s */
#define DERIVATIVE_GAIN                    4.0f   /* s   */
#define DERIVATIVE_CLAMP_C_PER_SECOND      5.0f
#define HEATR_CHANNEL_1_PIN    (7u)
#define HEATR_CHANNEL_2_PIN    (15u)
#define ADS7142_RDY_PIN        (5u)
#define MAX_DUTY               ((1 << 13) - 1)

#define ALPHA_NUM   1      //  numerator of α  (1…255)  -> α = 1/16 ≈ 0.0625
#define ALPHA_DEN   16     //  denominator of α
#define DEADBAND    4      //  optional ±LSB band after EMA (0 to disable)


static struct {
    struct arg_int *temperature;
    struct arg_end *end;
} pidtest_args;

ledc_channel_config_t heater_channel_1 = {
    .channel    = LEDC_CHANNEL_2,
    .duty       = 0,
    .gpio_num   = HEATR_CHANNEL_1_PIN,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .hpoint     = 0,
    .timer_sel  = LEDC_TIMER_3,
    .flags.output_invert = 0
};

ledc_channel_config_t heater_channel_2 = {
    .channel    = LEDC_CHANNEL_3,
    .duty       = 0,
    .gpio_num   = HEATR_CHANNEL_2_PIN,
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

static void register_pidtest(void);
double calculate_rntc(double Vout);
double calculate_temperature_celsius(double Rntc);
uint16_t filter_adc(uint16_t raw_adc);

int userSetPointCelsius  = 0; /* Desired board temperature */
static PID_Controller heaterController;
extern i2c_master_dev_handle_t ADC_handle;

static uint16_t med_buf[3] = {0};
static size_t   med_idx = 0;
static uint32_t ema_acc  = 0;      // accumulator in Q0 format (same units as ADC)
static uint16_t last_out = 0;      // after dead‑band

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

    printf("P:%.2f I:%.2f D:%.2f\r\n ", proportionalOutput, integralOutput, derivativeOutput);

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

float read_board_temperature_celsius()
{
    uint8_t cmd[2] = {0x00};

    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_START_SEQUENCE;
    cmd[2] = ADS7142_VAL_START_SEQUENCE;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));

    // call count
    while(gpio_get_level(ADS7142_RDY_PIN));

    //Read the MSB of Ch0 Accumulated Data after 16 accumulations are complete
    uint8_t accch0MSB = 0;
    cmd[0] = SINGLE_REG_READ;
    cmd[1] = ADS7142_REG_ACC_CH0_MSB;
    i2c_master_transmit_receive(ADC_handle, cmd, 2, &accch0MSB, 1, -1);

    //Read the LSB of Ch0 Accumulated Data after 16 accumulations are complete
    uint8_t accch0LSB = 0;
    cmd[0] = SINGLE_REG_READ;
    cmd[1] = ADS7142_REG_ACC_CH0_LSB;
    i2c_master_transmit_receive(ADC_handle, cmd, 2, &accch0LSB, 1, -1);

    //Read the MSB of Ch1 Accumulated Data after 16 accumulations are complete
    uint8_t accch1MSB = 0;
    cmd[0] = SINGLE_REG_READ;
    cmd[1] = ADS7142_REG_ACC_CH1_MSB;
    i2c_master_transmit_receive(ADC_handle, cmd, 2, &accch1MSB, 1, -1);

    //Read the LSB of Ch1 Accumulated Data after 16 accumulations are complete
    uint8_t accch1LSB = 0;
    cmd[0] = SINGLE_REG_READ;
    cmd[1] = ADS7142_REG_ACC_CH1_LSB;
    i2c_master_transmit_receive(ADC_handle, cmd, 2, &accch1LSB, 1, -1); 

    uint16_t ch1 = ((uint16_t)accch1MSB << 8) | accch1LSB;

    //ch1 = filter_adc(ch1);
   
    double voltage = VALUE_PER_BIT * (double)ch1;
    
    double Rntc = calculate_rntc(voltage);

    double tempC = calculate_temperature_celsius(Rntc);

    return tempC;
}

void pidtest_setup(void)
{
    ledc_timer_config_t heater_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,    // timer mode
        .timer_num = LEDC_TIMER_3,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };

    ledc_timer_config(&heater_timer);

    ledc_channel_config(&heater_channel_1);
    ledc_channel_config(&heater_channel_2);

    ledc_set_duty(heater_channel_1.speed_mode, heater_channel_1.channel, 0);
    ledc_update_duty(heater_channel_1.speed_mode, heater_channel_1.channel);

    ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, 0);
    ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);

    register_pidtest();
}

static int do_pidtest_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&pidtest_args);
    
    if (nerrors != 0) {
        arg_print_errors(stderr, pidtest_args.end, argv[0]);
        return 0;
    }

    userSetPointCelsius = pidtest_args.temperature->ival[0];

    PIDController_Init(&heaterController,
                       PROPORTIONAL_GAIN,
                       INTEGRAL_GAIN,
                       DERIVATIVE_GAIN,
                       0.0f,
                       100.0f);

    uint32_t previousMillis = (xTaskGetTickCount() * portTICK_PERIOD_MS);

    while (1) 
    {
        // Prepare select() to check for input
        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        // Non-blocking check for input
        int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

        if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds))
        {
            // Input available, read it
            char *line = linenoise("");

            if (line != NULL) 
            {
                printf("Exit PID test.\r\n");
                linenoiseFree(line);

                ledc_set_duty(heater_channel_1.speed_mode, heater_channel_1.channel, 0);
                ledc_update_duty(heater_channel_1.speed_mode, heater_channel_1.channel);

                ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, 0);
                ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);
                break;
            }
        }
    
        uint32_t currentMillis = (xTaskGetTickCount() * portTICK_PERIOD_MS);

        /* Run controller at fixed interval */
        if ((currentMillis - previousMillis) > SAMPLING_PERIOD_MILLISECONDS) 
        {
            float deltaTimeSeconds = (currentMillis - previousMillis) / 1000.0f;
            previousMillis = currentMillis;

            /* ---- Sensor read ---- */
            float boardTemperatureCelsius = read_board_temperature_celsius();
            printf("Target: %u Temp:|%.2f| - ", userSetPointCelsius, boardTemperatureCelsius);

            /* ---- Safety check ---- */
            if (boardTemperatureCelsius >= BOARD_MAX_TEMPERATURE_C) 
            {
                ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, 0);
                ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);
                return 0;
            }

            /* ---- PID computation ---- */
            float dutyPercent = PIDController_Compute(&heaterController,
                                                    userSetPointCelsius,
                                                    boardTemperatureCelsius,
                                                    deltaTimeSeconds);
            
            printf("PWM: %.2f\r\n ", dutyPercent);

            /* ---- Actuator update ---- */
            ledc_set_duty(heater_channel_2.speed_mode, heater_channel_2.channel, MAX_DUTY*dutyPercent/100);
            ledc_update_duty(heater_channel_2.speed_mode, heater_channel_2.channel);
        }
    }
    
    return 0;
}

static void register_pidtest(void)
{
    pidtest_args.temperature = arg_int1("c", "temperature", "<temperature>", "Temperature in C.");
    pidtest_args.end = arg_end(1);

    const esp_console_cmd_t pidtest_cmd = {
        .command = "pidtest",
        .help = "Run the heater using PID controller.",
        .hint = NULL,
        .func = &do_pidtest_cmd,
        .argtable = &pidtest_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&pidtest_cmd));
}

/// Call once per new raw 14‑bit ADC reading
uint16_t filter_adc(uint16_t raw_adc)
{
    /* ---------- Median‑of‑3 ---------- */
    med_buf[med_idx] = raw_adc;
    med_idx = (med_idx + 1) % 3;

    uint16_t a = med_buf[0], b = med_buf[1], c = med_buf[2];
    uint16_t med;

    // Find the median of a, b, c
    if ((a >= b && a <= c) || (a <= b && a >= c)) {
        med = a;
    } else if ((b >= a && b <= c) || (b <= a && b >= c)) {
        med = b;
    } else {
        med = c;
    }

    /* ---------- Exponential Moving Average ----------
       ema_acc = ema_acc + α*(med - ema_acc)
       Implemented with integer math:     */
    ema_acc += ((uint32_t)ALPHA_NUM * (med - ema_acc) + (ALPHA_DEN/2)) / ALPHA_DEN;
    uint16_t ema_out = (uint16_t)ema_acc;   // convert back to 16‑bit

    /* ---------- Optional dead‑band ---------- */
    if (DEADBAND == 0)
        return ema_out;

    if (abs((int)ema_out - (int)last_out) > DEADBAND)
        last_out = ema_out;

    return last_out;
}
