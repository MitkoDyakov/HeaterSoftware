#include "orientation.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "pinout.h"
#include "switchboard/switchboard.h" // for button definitions

// ---------------- Display Tilt Detection ----------------
static lv_display_t *s_lvgl_display = NULL;         // LVGL display reference
static esp_lcd_panel_handle_t s_panel_handle = NULL; // LCD panel reference
static bool s_display_flipped = false;               // Current orientation state
static TickType_t s_last_tilt_change = 0;           // Debounce timing
static bool s_last_tilt_state = false;              // Last stable tilt pin state

#define TILT_DEBOUNCE_MS 50  // Debounce time in milliseconds

static void display_set_orientation(bool flipped) {
    if (!s_panel_handle || !s_lvgl_display) return;
    
    if (s_display_flipped == flipped) return; // No change needed
    
    s_display_flipped = flipped;
    
    if (flipped) {
        // 180° rotated orientation (270° LVGL rotation + Y-axis mirror)
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(s_panel_handle, false, true));
        lv_display_set_rotation(s_lvgl_display, LV_DISPLAY_ROTATION_270);
    } else {
        // Normal orientation (90° LVGL rotation + X-axis mirror)
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(s_panel_handle, true, false));
        lv_display_set_rotation(s_lvgl_display, LV_DISPLAY_ROTATION_90);
    }
    
    ESP_LOGI("orientation", "Display orientation: %s", flipped ? "flipped" : "normal");
}

void orientation_init(lv_display_t *display, esp_lcd_panel_handle_t panel) {
    s_lvgl_display = display;
    s_panel_handle = panel;
    
    // Configure tilt detection GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << DISPLAY_TILT),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf);
    
    // Initialize tilt state
    s_last_tilt_state = gpio_get_level(DISPLAY_TILT);
    s_last_tilt_change = xTaskGetTickCount();
    
    // Set initial orientation based on tilt pin state
    s_display_flipped = false; // Force update
    // Invert logic: LOW (ground) = flipped orientation, HIGH (pull-up) = normal
    display_set_orientation(!s_last_tilt_state);
    
    ESP_LOGI("orientation", "Tilt GPIO initialized (pin %d, pull-up enabled, initial state: %d)", 
             DISPLAY_TILT, s_last_tilt_state);
}

void orientation_check_and_update(void) {
    if (!s_lvgl_display) return;
    
    bool current_state = gpio_get_level(DISPLAY_TILT);
    TickType_t now = xTaskGetTickCount();
    
    // Check if state changed
    if (current_state != s_last_tilt_state) {
        s_last_tilt_change = now;
        s_last_tilt_state = current_state;
        return; // Wait for debounce
    }
    
    // Check if debounce time has passed
    TickType_t elapsed = now - s_last_tilt_change;
    if (elapsed < pdMS_TO_TICKS(TILT_DEBOUNCE_MS)) {
        return; // Still in debounce period
    }
    
    // State is stable, update display orientation
    // Invert logic: LOW (ground) = flipped orientation, HIGH (pull-up) = normal
    display_set_orientation(!current_state);
}

uint8_t orientation_remap_button(uint8_t original_btn_id) {
    ESP_LOGI("orientation", "button %u -> ", original_btn_id);

    if (!s_display_flipped) {
        return original_btn_id; // No remapping needed for normal orientation
    }

    // When display is rotated 180°, remap buttons to maintain logical positions
    // Physical button -> Logical position after 180° rotation
    switch (original_btn_id) {
        case BUTTON_RIGHT_TOP:    return BUTTON_LEFT_BOTTOM;
        case BUTTON_RIGHT_CENTER: return BUTTON_LEFT_CENTER;
        case BUTTON_RIGHT_BOTTOM: return BUTTON_LEFT_TOP;
        case BUTTON_LEFT_TOP:     return BUTTON_RIGHT_BOTTOM;
        case BUTTON_LEFT_CENTER:  return BUTTON_RIGHT_CENTER;
        case BUTTON_LEFT_BOTTOM:  return BUTTON_RIGHT_TOP;
        default: 
            ESP_LOGW("orientation", "Unknown button ID %d, no remapping", original_btn_id);
            return original_btn_id;
    }
}

bool orientation_is_flipped(void) {
    return s_display_flipped;
}