#include "composer.h"
#include "pinout.h"  // for BUZZER pin macro
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "pwm_alloc.h"
#include "wiseman/wiseman.h"  // for sound_enabled setting

#define COMPOSER_LEDC_TIMER      PWM_BUZZER_TIMER
#define COMPOSER_LEDC_MODE       LEDC_LOW_SPEED_MODE
#define COMPOSER_LEDC_CHANNEL    PWM_BUZZER_CHANNEL
#define COMPOSER_LEDC_DUTY_RES   LEDC_TIMER_10_BIT  // 10-bit resolution
#define COMPOSER_FREQUENCY_HZ    4000               // 4 kHz
#define COMPOSER_SHORT_BEEP_MS   150                // Short beep duration
#define COMPOSER_LONG_BEEP_MS    1500               // Long beep duration
#define COMPOSER_ALARM_PERIOD_MS 1000               // Alarm on/off period

static TimerHandle_t composer_timer = NULL;
static SemaphoreHandle_t composer_mutex = NULL; // protect state
static enum { COMPOSER_IDLE, COMPOSER_ONESHOT, COMPOSER_ALARM_ON, COMPOSER_ALARM_OFF } composer_state = COMPOSER_IDLE;
static const wiseman_settings_t* g_settings = NULL; // global reference to wiseman settings

static void composer_set(bool on) {
    uint32_t duty = on ? (1 << (COMPOSER_LEDC_DUTY_RES - 1)) : 0; // 50% duty cycle or off
    ledc_set_duty(COMPOSER_LEDC_MODE, COMPOSER_LEDC_CHANNEL, duty);
    ledc_update_duty(COMPOSER_LEDC_MODE, COMPOSER_LEDC_CHANNEL);
}

static void composer_timer_cb(TimerHandle_t t) {
    
    if (xSemaphoreTake(composer_mutex, 0) != pdTRUE) {
        return; // couldn't lock, skip this tick (should be rare)
    }
    
    switch (composer_state) {
        case COMPOSER_ONESHOT:
            // oneshot complete
            composer_set(false);
            composer_state = COMPOSER_IDLE;
            break;
        case COMPOSER_ALARM_ON:
            // finished ON period -> go OFF for 1s
            composer_set(false);
            composer_state = COMPOSER_ALARM_OFF;
            xTimerChangePeriod(composer_timer, pdMS_TO_TICKS(COMPOSER_ALARM_PERIOD_MS), 0);
            break;
        case COMPOSER_ALARM_OFF:
            // finished OFF period -> go ON for 1s
            composer_set(true);
            composer_state = COMPOSER_ALARM_ON;
            xTimerChangePeriod(composer_timer, pdMS_TO_TICKS(COMPOSER_ALARM_PERIOD_MS), 0);
            break;
        default:
            break;
    }

    xSemaphoreGive(composer_mutex);
}

esp_err_t composer_init(void) {

    if (composer_timer == NULL) {
        composer_timer = xTimerCreate("cmp", pdMS_TO_TICKS(COMPOSER_ALARM_PERIOD_MS), pdFALSE, NULL, composer_timer_cb);
        if (!composer_timer) return ESP_ERR_NO_MEM;
    }

    if (composer_mutex == NULL) {
        composer_mutex = xSemaphoreCreateMutex();
        if (!composer_mutex) return ESP_ERR_NO_MEM;
    }

    // (Re)configure LEDC each call; inexpensive and idempotent for simple init.

    ledc_timer_config_t timer_cfg = {
        .speed_mode       = COMPOSER_LEDC_MODE,
        .duty_resolution  = COMPOSER_LEDC_DUTY_RES,
        .timer_num        = COMPOSER_LEDC_TIMER,
        .freq_hz          = COMPOSER_FREQUENCY_HZ,
        .clk_cfg          = LEDC_AUTO_CLK,
        .deconfigure = false,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    ledc_channel_config_t ch_cfg = {
        .gpio_num   = BUZZER,
        .speed_mode = COMPOSER_LEDC_MODE,
        .channel    = COMPOSER_LEDC_CHANNEL,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = COMPOSER_LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0,
        .flags.output_invert = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg));

    composer_set(false);
    composer_state = COMPOSER_IDLE;
    
    // Get reference to wiseman settings (updated automatically when settings change)
    g_settings = wiseman_get();
    
    // Startup beep to confirm composer is working (if sound enabled)
    composer_short_beep();
    
    return ESP_OK;
}

static esp_err_t composer_start_oneshot_ms(uint32_t ms) {
    // Check if sound is enabled (beeps only, not alarms)
    if (g_settings->sound_enabled == false) {
        return ESP_OK; // Sound disabled, silently succeed
    }
    
    if (xSemaphoreTake(composer_mutex, pdMS_TO_TICKS(10)) != pdTRUE)
    {
         return ESP_ERR_TIMEOUT;
    }

    // Do not override active alarm pattern
    if (composer_state == COMPOSER_ALARM_ON || composer_state == COMPOSER_ALARM_OFF) {
        xSemaphoreGive(composer_mutex);
        return ESP_ERR_INVALID_STATE;
    }

    composer_set(true);
    composer_state = COMPOSER_ONESHOT;
    xTimerStop(composer_timer, 0);
    xTimerChangePeriod(composer_timer, pdMS_TO_TICKS(ms), 0);
    xTimerStart(composer_timer, 0);
    xSemaphoreGive(composer_mutex);
    return ESP_OK;
}

esp_err_t composer_short_beep(void) {
    return composer_start_oneshot_ms(COMPOSER_SHORT_BEEP_MS);
}

esp_err_t composer_long_beep(void) {
    return composer_start_oneshot_ms(COMPOSER_LONG_BEEP_MS);
}

esp_err_t composer_alarm_start(void) {

    if (xSemaphoreTake(composer_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    if (composer_state == COMPOSER_ALARM_ON || composer_state == COMPOSER_ALARM_OFF) {
        xSemaphoreGive(composer_mutex);
        return ESP_OK; // already running
    }
    xTimerStop(composer_timer, 0);
    composer_set(true);
    composer_state = COMPOSER_ALARM_ON;
    xTimerChangePeriod(composer_timer, pdMS_TO_TICKS(COMPOSER_ALARM_PERIOD_MS), 0);
    xTimerStart(composer_timer, 0);
    xSemaphoreGive(composer_mutex);
    return ESP_OK;
}

void composer_sound_off(void) {

    if (xSemaphoreTake(composer_mutex, pdMS_TO_TICKS(20)) != pdTRUE)
    {
        return;
    }

    if (composer_timer) {
        xTimerStop(composer_timer, 0);
    }

    composer_set(false);
    composer_state = COMPOSER_IDLE;
    xSemaphoreGive(composer_mutex);
}
