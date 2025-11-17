#include "backlight.h"
#include "driver/ledc.h"
#include "pwm_alloc.h"
#include "pinout.h"

// ---------------- Backlight PWM (LEDC) Configuration ----------------
#define BACKLIGHT_LEDC_TIMER       PWM_BACKLIGHT_TIMER
#define BACKLIGHT_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BACKLIGHT_LEDC_CHANNEL     PWM_BACKLIGHT_CHANNEL
#define BACKLIGHT_LEDC_DUTY_RES    LEDC_TIMER_10_BIT   // 0..1023
#define BACKLIGHT_LEDC_FREQ_HZ     5000                // 5 kHz (no flicker)

static uint16_t s_backlight_max_duty = (1u << 10) - 1; // 1023 for 10-bit
static uint8_t  s_backlight_last_pct = 0xFF;           // force initial apply

void backlight_init(uint8_t initial_brightness) {
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