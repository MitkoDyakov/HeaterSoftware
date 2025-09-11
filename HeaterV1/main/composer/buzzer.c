#include "buzzer.h"
#include "pinout.h"  // for BUZZER pin macro
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "pwm_alloc.h"
#define BUZZER_LEDC_TIMER      PWM_BUZZER_TIMER
#define BUZZER_LEDC_MODE       LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL    PWM_BUZZER_CHANNEL
#define BUZZER_LEDC_DUTY_RES   LEDC_TIMER_10_BIT  // 10-bit resolution
#define BUZZER_FREQUENCY_HZ    4000               // 4 kHz
#define BUZZER_DUTY_PERCENT    50                 // square wave

static TimerHandle_t buzzer_timer = NULL;
static SemaphoreHandle_t buzzer_mutex = NULL; // protect state
static enum { BUZZER_IDLE, BUZZER_ONESHOT, BUZZER_ALARM_ON, BUZZER_ALARM_OFF } buzzer_state = BUZZER_IDLE;

static void buzzer_set(bool on) {
    uint32_t duty = on ? ((1 << BUZZER_LEDC_DUTY_RES) * BUZZER_DUTY_PERCENT) / 100 : 0;
    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, duty);
    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
}

static void buzzer_timer_cb(TimerHandle_t t) {
    if (buzzer_mutex && xSemaphoreTake(buzzer_mutex, 0) != pdTRUE) {
        return; // couldn't lock, skip this tick (should be rare)
    }
    switch (buzzer_state) {
    case BUZZER_ONESHOT:
        // oneshot complete
        buzzer_set(false);
        buzzer_state = BUZZER_IDLE;
        break;
    case BUZZER_ALARM_ON:
        // finished ON period -> go OFF for 1s
        buzzer_set(false);
        buzzer_state = BUZZER_ALARM_OFF;
        xTimerChangePeriod(buzzer_timer, pdMS_TO_TICKS(1000), 0);
        break;
    case BUZZER_ALARM_OFF:
        // finished OFF period -> go ON for 1s
        buzzer_set(true);
        buzzer_state = BUZZER_ALARM_ON;
        xTimerChangePeriod(buzzer_timer, pdMS_TO_TICKS(1000), 0);
        break;
    default:
        break;
    }
    if (buzzer_mutex) xSemaphoreGive(buzzer_mutex);
}

esp_err_t buzzer_init(void) {
    if (buzzer_timer == NULL) {
        buzzer_timer = xTimerCreate("bzz", pdMS_TO_TICKS(1000), pdFALSE, NULL, buzzer_timer_cb);
        if (!buzzer_timer) return ESP_ERR_NO_MEM;
    }
    if (buzzer_mutex == NULL) {
        buzzer_mutex = xSemaphoreCreateMutex();
        if (!buzzer_mutex) return ESP_ERR_NO_MEM;
    }

    // (Re)configure LEDC each call; inexpensive and idempotent for simple init.

    ledc_timer_config_t timer_cfg = {
        .speed_mode       = BUZZER_LEDC_MODE,
        .duty_resolution  = BUZZER_LEDC_DUTY_RES,
        .timer_num        = BUZZER_LEDC_TIMER,
        .freq_hz          = BUZZER_FREQUENCY_HZ,
        .clk_cfg          = LEDC_AUTO_CLK,
        .deconfigure = false,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    ledc_channel_config_t ch_cfg = {
        .gpio_num   = BUZZER,
        .speed_mode = BUZZER_LEDC_MODE,
        .channel    = BUZZER_LEDC_CHANNEL,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = BUZZER_LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
        .flags.output_invert = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg));

    buzzer_set(false);
    buzzer_state = BUZZER_IDLE;
    return ESP_OK;
}

static esp_err_t buzzer_start_oneshot_ms(uint32_t ms) {
    if (!buzzer_mutex || xSemaphoreTake(buzzer_mutex, pdMS_TO_TICKS(10)) != pdTRUE) return ESP_ERR_TIMEOUT;
    // Do not override active alarm pattern
    if (buzzer_state == BUZZER_ALARM_ON || buzzer_state == BUZZER_ALARM_OFF) {
        xSemaphoreGive(buzzer_mutex);
        return ESP_ERR_INVALID_STATE;
    }
    buzzer_set(true);
    buzzer_state = BUZZER_ONESHOT;
    xTimerStop(buzzer_timer, 0);
    xTimerChangePeriod(buzzer_timer, pdMS_TO_TICKS(ms), 0);
    xTimerStart(buzzer_timer, 0);
    xSemaphoreGive(buzzer_mutex);
    return ESP_OK;
}

esp_err_t buzzer_short_beep(void) { return buzzer_start_oneshot_ms(150); }

esp_err_t buzzer_long_beep(void) { return buzzer_start_oneshot_ms(1500); }

esp_err_t buzzer_alarm_start(void) {
    if (!buzzer_mutex || xSemaphoreTake(buzzer_mutex, pdMS_TO_TICKS(10)) != pdTRUE) return ESP_ERR_TIMEOUT;
    if (buzzer_state == BUZZER_ALARM_ON || buzzer_state == BUZZER_ALARM_OFF) {
        xSemaphoreGive(buzzer_mutex);
        return ESP_OK; // already running
    }
    xTimerStop(buzzer_timer, 0);
    buzzer_set(true);
    buzzer_state = BUZZER_ALARM_ON;
    xTimerChangePeriod(buzzer_timer, pdMS_TO_TICKS(1000), 0);
    xTimerStart(buzzer_timer, 0);
    xSemaphoreGive(buzzer_mutex);
    return ESP_OK;
}

void buzzer_stop(void) {
    if (!buzzer_mutex || xSemaphoreTake(buzzer_mutex, pdMS_TO_TICKS(20)) != pdTRUE) return;
    if (buzzer_timer) {
        xTimerStop(buzzer_timer, 0);
    }
    buzzer_set(false);
    buzzer_state = BUZZER_IDLE;
    xSemaphoreGive(buzzer_mutex);
}
