#ifndef BUZZER_H
#define BUZZER_H

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize buzzer (GPIO and PWM) on fixed BUZZER pin (pinout.h).
esp_err_t buzzer_init(void);

// Play a short beep (0.5s at 2kHz)
esp_err_t buzzer_short_beep(void);

// Play a long beep (1.5s at 2kHz)
esp_err_t buzzer_long_beep(void);

// Start alarm pattern: 1s on, 1s off repeating until buzzer_alarm_stop() called
esp_err_t buzzer_alarm_start(void);

// Stop any current sound/alarm
void buzzer_stop(void);

// Stop alarm pattern (alias of buzzer_stop for clarity)
static inline void buzzer_alarm_stop(void) { buzzer_stop(); }

#ifdef __cplusplus
}
#endif

#endif // BUZZER_H
