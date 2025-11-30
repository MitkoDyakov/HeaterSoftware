#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize the display backlight PWM controller
 * 
 * Configures LEDC timer and channel for backlight control.
 * Hardware note: GPIO high turns display OFF (inverted logic).
 * Reads initial brightness from wiseman persistent settings.
 */
void backlight_init(void);

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

/**
 * @brief Initialize display sleep timer (must be called after LVGL init)
 * 
 * Creates LVGL timer that monitors inactivity and auto-dims display.
 * Reads sleep timeout from sleepTimer LVGL subject.
 */
void backlight_init_sleep_timer(void);

/**
 * @brief Signal user activity to prevent/wake from display sleep
 * 
 * Call this on any button press or user interaction.
 * Wakes display if sleeping and resets inactivity timer.
 * 
 * @return true if display was sleeping and is now awake, false otherwise
 */
bool backlight_activity(void);

#endif // BACKLIGHT_H