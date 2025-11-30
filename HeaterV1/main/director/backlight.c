#include "backlight.h"
#include "driver/ledc.h"
#include "pwm_alloc.h"
#include "pinout.h"
#include "wiseman/wiseman.h"
#include "lvgl.h"
#include "HeaterGUI_gen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

// ---------------- Backlight PWM (LEDC) Configuration ----------------
#define BACKLIGHT_LEDC_TIMER       PWM_BACKLIGHT_TIMER
#define BACKLIGHT_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BACKLIGHT_LEDC_CHANNEL     PWM_BACKLIGHT_CHANNEL
#define BACKLIGHT_LEDC_DUTY_RES    LEDC_TIMER_10_BIT   // 0..1023
#define BACKLIGHT_LEDC_FREQ_HZ     5000                // 5 kHz (no flicker)

static uint16_t s_backlight_max_duty = (1u << 10) - 1; // 1023 for 10-bit
static uint8_t  s_backlight_last_pct = 0xFF;           // force initial apply

// Display sleep state
static bool s_display_sleeping = false;       // true when dimmed due to inactivity
static TickType_t s_last_input_tick = 0;      // last button activity (for inactivity timer)
static lv_timer_t *s_sleep_check_timer = NULL; // checks inactivity for display sleep

void backlight_init(void) {
    // Configure timer
    const ledc_timer_config_t timer_cfg = {
        .speed_mode       = BACKLIGHT_LEDC_MODE,
        .duty_resolution  = BACKLIGHT_LEDC_DUTY_RES,
        .timer_num        = BACKLIGHT_LEDC_TIMER,
        .freq_hz          = BACKLIGHT_LEDC_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_cfg);

    // Configure channel
    const ledc_channel_config_t ch_cfg = {
        .gpio_num       = DISPLAY_BACKLIGHT,
        .speed_mode     = BACKLIGHT_LEDC_MODE,
        .channel        = BACKLIGHT_LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = BACKLIGHT_LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0,
        .flags.output_invert = 0
    };
    ledc_channel_config(&ch_cfg);

    s_backlight_last_pct = 0xFF; // ensure set
    
    // Read initial brightness from wiseman settings
    uint8_t initial_brightness = 100; // fallback
    const wiseman_settings_t *ws = wiseman_get();
    if (ws) initial_brightness = ws->display_brightness;
    
    // Enforce minimum so user cannot soft-brick UI visibility
    if (initial_brightness < 5) {
        initial_brightness = 5;
        wiseman_set_display_brightness(initial_brightness);
    }
    
    // Apply initial brightness
    backlight_set_brightness(initial_brightness);
}

void backlight_set_brightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    if (brightness == s_backlight_last_pct) return; // no change
    
    // Hardware note: GPIO high turns display OFF; invert duty mapping
    uint32_t duty = ((uint32_t)(100 - brightness) * s_backlight_max_duty) / 100;
    ledc_set_duty(BACKLIGHT_LEDC_MODE, BACKLIGHT_LEDC_CHANNEL, duty);
    ledc_update_duty(BACKLIGHT_LEDC_MODE, BACKLIGHT_LEDC_CHANNEL);
    s_backlight_last_pct = brightness;
}

uint8_t backlight_get_brightness(void) {
    return s_backlight_last_pct == 0xFF ? 0 : s_backlight_last_pct;
}

// Apply the user's configured brightness from LVGL subject to hardware
static void apply_user_brightness(void) {
    int b = lv_subject_get_int(&brightness);
    if (b < 5) b = 5; else if (b > 100) b = 100; // user brightness minimum
    backlight_set_brightness((uint8_t)b);
}

// Periodically check inactivity and dim the display according to sleepTimer
static void sleep_check_cb(lv_timer_t *t) {
    (void)t;
    int st = lv_subject_get_int(&sleepTimer); // seconds; 0=OFF
    if (st <= 0) {
        // Sleep disabled; ensure we're awake
        if (s_display_sleeping) {
            apply_user_brightness(); // already clamps to >=5
            s_display_sleeping = false;
        }
        return;
    }

    TickType_t now = xTaskGetTickCount();
    TickType_t idle_ticks = now - s_last_input_tick;
    uint32_t idle_ms = (uint32_t)idle_ticks * portTICK_PERIOD_MS;

    if (!s_display_sleeping) {
        if (idle_ms >= (uint32_t)st * 1000U) {
            // Dim display (do not change UI subject so user brightness is preserved)
            backlight_set_brightness(0);
            s_display_sleeping = true;
        }
    }
}

void backlight_init_sleep_timer(void) {
    // Initialize last input time and create inactivity sleep checker
    s_last_input_tick = xTaskGetTickCount();
    s_sleep_check_timer = lv_timer_create(sleep_check_cb, 250, NULL);
}

bool backlight_activity(void) {
    // Update inactivity timer
    s_last_input_tick = xTaskGetTickCount();
    
    // Wake display if sleeping
    if (s_display_sleeping) {
        apply_user_brightness();
        s_display_sleeping = false;
        return true; // was sleeping, now awake
    }
    
    return false; // was already awake
}