#ifndef ORIENTATION_H
#define ORIENTATION_H

#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"

/**
 * @brief Initialize display orientation detection and control
 * 
 * Sets up GPIO for tilt detection and stores references to display objects
 * for runtime orientation changes.
 * 
 * @param display LVGL display object for rotation control
 * @param panel LCD panel handle for mirror control  
 */
void orientation_init(lv_display_t *display, esp_lcd_panel_handle_t panel);

/**
 * @brief Check tilt sensor and update display orientation if needed
 * 
 * Should be called periodically from main loop. Handles debouncing
 * and applies orientation changes when tilt state is stable.
 */
void orientation_check_and_update(void);

/**
 * @brief Remap button ID based on current display orientation
 * 
 * When display is rotated 180°, physical buttons need logical remapping
 * to maintain intuitive user experience.
 * 
 * @param original_btn_id Physical button ID from hardware
 * @return Logical button ID after orientation remapping
 */
uint8_t orientation_remap_button(uint8_t original_btn_id);

/**
 * @brief Check if display is currently in flipped orientation
 * 
 * @return true if display is rotated 180°, false if normal
 */
bool orientation_is_flipped(void);

#endif // ORIENTATION_H