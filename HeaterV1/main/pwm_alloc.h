#pragma once
/* Central allocation of LEDC timers/channels to avoid conflicts.
 * All modules should include this header instead of hardcoding timers/channels.
 * Using LOW_SPEED mode for everything for simplicity; adjust if needed.
 */
#include "driver/ledc.h"

/* Buzzer: simple square wave */
#define PWM_BUZZER_TIMER      LEDC_TIMER_0
#define PWM_BUZZER_CHANNEL    LEDC_CHANNEL_0

/* Backlight: 5 kHz dimming */
#define PWM_BACKLIGHT_TIMER   LEDC_TIMER_1
#define PWM_BACKLIGHT_CHANNEL LEDC_CHANNEL_1

/* Heaters: shared timer, two channels (higher resolution 13-bit) */
#define PWM_HEATER_TIMER      LEDC_TIMER_3
#define PWM_HEATER_CH1        LEDC_CHANNEL_2
#define PWM_HEATER_CH2        LEDC_CHANNEL_3

/* Sanity static assertions (channels must all differ since we use same speed mode) */
_Static_assert(PWM_BUZZER_CHANNEL != PWM_BACKLIGHT_CHANNEL, "Buzzer and backlight channel conflict");
_Static_assert(PWM_BUZZER_CHANNEL != PWM_HEATER_CH1 && PWM_BUZZER_CHANNEL != PWM_HEATER_CH2, "Buzzer/heater channel conflict");
_Static_assert(PWM_BACKLIGHT_CHANNEL != PWM_HEATER_CH1 && PWM_BACKLIGHT_CHANNEL != PWM_HEATER_CH2, "Backlight/heater channel conflict");

/* Timer choices may repeat across peripherals only if you intend to share frequency/resolution.
 * Currently buzzer/backlight/heaters all use distinct timers for flexibility.
 */
