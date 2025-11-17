#ifndef COMPOSER_H
#define COMPOSER_H

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize composer (GPIO and PWM) on fixed BUZZER pin (pinout.h).
esp_err_t composer_init(void);

// Play a short beep (0.5s at 2kHz)
esp_err_t composer_short_beep(void);

// Play a long beep (1.5s at 2kHz)
esp_err_t composer_long_beep(void);

// Start alarm pattern: 1s on, 1s off repeating until composer_sound_off() called
esp_err_t composer_alarm_start(void);

// Stop any current sound/alarm
void composer_sound_off(void);

#ifdef __cplusplus
}
#endif

#endif // COMPOSER_H
