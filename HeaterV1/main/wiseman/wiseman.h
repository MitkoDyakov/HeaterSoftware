#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Versioning for settings struct; bump when format changes.
#define WISEMAN_SETTINGS_VERSION 2

// Auto-save debounce for setpoints (seconds) default
#ifndef WISEMAN_SETPOINT_AUTOSAVE_SECS_DEFAULT
#define WISEMAN_SETPOINT_AUTOSAVE_SECS_DEFAULT 30
#endif

// Settings structure; add fields as needed. Keep packed/stable.
typedef struct {
    uint32_t version;         // must equal WISEMAN_SETTINGS_VERSION
    // User-adjustable parameters
    int16_t setpoint1_c;      // heater 1 setpoint in C
    int16_t setpoint2_c;      // heater 2 setpoint in C
    bool heater1_enabled;
    bool heater2_enabled;
    bool sound_enabled;       // buzzer on/off
    uint8_t display_brightness; // 0-100 percent
    char wifi_ssid[33];       // null-terminated
    char wifi_pass[65];       // null-terminated
    uint32_t op_time_min;     // accumulated operating time (minutes)
    // Add more as required
} wiseman_settings_t;

// Initialize persistence (NVS), load settings or defaults, and start autosave timer.
bool wiseman_init(void);

// Accessors
const wiseman_settings_t* wiseman_get(void);
// Thread-safe copy of current settings into caller-provided buffer
bool wiseman_get_copy(wiseman_settings_t* out);

// Operating time accessors
uint32_t wiseman_get_op_time_minutes(void);
void wiseman_add_op_time_minutes(uint32_t minutes); // add (saturating) and optionally persist later

// Mutators: mark-dirty on change and schedule auto-save
void wiseman_set_setpoint1(int16_t c);
void wiseman_set_setpoint2(int16_t c);
void wiseman_set_sound_enabled(bool en);
void wiseman_set_display_brightness(uint8_t pct);
void wiseman_set_wifi_credentials(const char* ssid, const char* pass);

// Manual save: persist current settings immediately
bool wiseman_save_now(void);

// Reset to compiled defaults and save
bool wiseman_reset_to_defaults(void);

// Configure setpoint autosave debounce timeout in seconds (0 disables autosave)
void wiseman_set_autosave_timeout(uint32_t seconds);

#ifdef __cplusplus
}
#endif
