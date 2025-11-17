#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <stdint.h>

/**
 * @brief Initialize the display backlight PWM controller
 * 
 * Configures LEDC timer and channel for backlight control.
 * Hardware note: GPIO high turns display OFF (inverted logic).
 * 
 * @param initial_brightness Initial brightness percentage (0-100)
 */
void backlight_init(uint8_t initial_brightness);

/**
 * @brief Set display backlight brightness
 * 
 * @param brightness Brightness percentage (0-100, values >100 clamped to 100)
 */
void backlight_set_brightness(uint8_t brightness);

/**
 * @brief Get current backlight brightness
 * 
 * @return Current brightness percentage (0-100)
 */
uint8_t backlight_get_brightness(void);

#endif // BACKLIGHT_H