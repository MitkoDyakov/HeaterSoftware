#include "wiseman.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "wiseman_persist.h"
#include <string.h>
#include <stdlib.h>

#define WISEMAN_NAMESPACE "wiseman"
#define WISEMAN_KEY       "settings"
static const char* TAG = "WISEMAN";

// In-RAM settings and defaults
static SemaphoreHandle_t s_mutex = NULL;
static wiseman_settings_t g_settings;
static wiseman_settings_t s_last_saved; // snapshot to avoid redundant writes
static bool s_have_snapshot = false;

static const wiseman_settings_t g_defaults = {
    .version = WISEMAN_SETTINGS_VERSION,
    .setpoint1_c = 30,
    .setpoint2_c = 30,
    .heater1_enabled = false,
    .heater2_enabled = false,
    .sound_enabled = true,
    .timer_mode = false,
    .preheat_min = 0,
    .display_brightness = 75,
    .wifi_ssid = "",
    .wifi_pass = "",
    .op_time_min = 0,
    .sleep_timeout_s = 30,
    .screen_orientation = WISEMAN_ORIENTATION_AUTO
};  

static bool load_from_nvs(void);

bool wiseman_init(void) {
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    
    ESP_ERROR_CHECK(err);

    if (!s_mutex) s_mutex = xSemaphoreCreateMutex();
    configASSERT(s_mutex);

    if (!load_from_nvs()) {
        ESP_LOGI(TAG, "No saved settings found, using defaults");
        (void)wiseman_save_now();
    } else {
        // Initialize snapshot with loaded data
        memcpy(&s_last_saved, &g_settings, sizeof(g_settings));
        s_have_snapshot = true;
    }

    // Start persistence task (idempotent)
    wiseman_persist_start();
    // No global autosave. Setpoints have their own debounce timer (except disabled in test).
    return true;
}

// get handle for nvs in read-write mode
static esp_err_t nvs_open_rw(nvs_handle_t* out) {
    nvs_handle_t temp_handle;

    esp_err_t err = nvs_open(WISEMAN_NAMESPACE, NVS_READWRITE, &temp_handle);    

    if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        // Should not happen if nvs_flash_init succeeds
        return err;
    }

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Namespace created on first write; treat as success by reopening
        err = nvs_open(WISEMAN_NAMESPACE, NVS_READWRITE, &temp_handle);
    }

    if (err == ESP_OK) {
        *out = temp_handle;
    } 

    return err;
}

static void apply_defaults(void) {
    memcpy(&g_settings, &g_defaults, sizeof(g_settings));
}

static bool load_from_nvs(void) {
    nvs_handle_t h;
    if (nvs_open_rw(&h) != ESP_OK) {
        return false;
    }
    
    // Start with known-good defaults first
    apply_defaults();
    
    // Try to load saved settings directly into g_settings
    size_t actual_size = sizeof(g_settings);
    esp_err_t err = nvs_get_blob(h, WISEMAN_KEY, &g_settings, &actual_size);
    nvs_close(h);
    
    if (err != ESP_OK) {
        // No saved settings or error - defaults already applied
        return false;
    }
    
    // Always ensure current version (for migration compatibility)
    g_settings.version = WISEMAN_SETTINGS_VERSION;
    return true;
}

bool wiseman_save_now(void) {
    // Copy current settings under lock, then perform flash I/O without holding mutex.
    wiseman_settings_t local_copy;
    
    xSemaphoreTake(s_mutex, portMAX_DELAY);

    if (s_have_snapshot && memcmp(&g_settings, &s_last_saved, sizeof(g_settings)) == 0) {
        xSemaphoreGive(s_mutex);
        return true; // nothing changed
    }

    memcpy(&local_copy, &g_settings, sizeof(local_copy));
    xSemaphoreGive(s_mutex);

    nvs_handle_t temp_handle;
    esp_err_t err = nvs_open_rw(&temp_handle);

    if (err != ESP_OK) {
        return false;
    }

    err = nvs_set_blob(temp_handle, WISEMAN_KEY, &local_copy, sizeof(local_copy));
    if (err == ESP_OK) {
        err = nvs_commit(temp_handle);
    } 
    nvs_close(temp_handle);

    if (err == ESP_OK) {
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        memcpy(&s_last_saved, &local_copy, sizeof(s_last_saved));
        s_have_snapshot = true;
        xSemaphoreGive(s_mutex);
    }

    return (err == ESP_OK);
}

bool wiseman_reset_to_defaults(void) {
    apply_defaults();
    return wiseman_save_now();
}

const wiseman_settings_t* wiseman_get(void) { 
    return &g_settings; 
}

bool wiseman_get_copy(wiseman_settings_t* out) {
    if (!out) return false;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    memcpy(out, &g_settings, sizeof(g_settings));
    xSemaphoreGive(s_mutex);
    return true;
}

void wiseman_set_setpoint1(int16_t c) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.setpoint1_c != c) {
        g_settings.setpoint1_c = c;
        wiseman_mark_dirty();
    }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_sleep_timeout_seconds(uint16_t seconds) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.sleep_timeout_s != seconds) {
        g_settings.sleep_timeout_s = seconds;
        wiseman_mark_dirty();
    }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_heaters_enabled(bool ch1, bool ch2) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    bool changed = false;
    if (g_settings.heater1_enabled != ch1) { g_settings.heater1_enabled = ch1; changed = true; }
    if (g_settings.heater2_enabled != ch2) { g_settings.heater2_enabled = ch2; changed = true; }
    xSemaphoreGive(s_mutex);
    if (changed) wiseman_mark_dirty();
}

void wiseman_set_setpoint2(int16_t c) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.setpoint2_c != c) {
        g_settings.setpoint2_c = c;
        wiseman_mark_dirty();
    }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_dual_setpoints(int16_t sp1, int16_t sp2) {
    wiseman_set_setpoint1(sp1);
    wiseman_set_setpoint2(sp2);
}

void wiseman_set_screen_orientation(wiseman_orientation_t orientation) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.screen_orientation != (uint8_t)orientation) {
        g_settings.screen_orientation = (uint8_t)orientation;
        wiseman_mark_dirty();
    }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_sound_enabled(bool en) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.sound_enabled != en) {
        g_settings.sound_enabled = en;
        wiseman_mark_dirty();
    }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_timer_mode(bool enabled) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.timer_mode != enabled) {
        g_settings.timer_mode = enabled;
        wiseman_mark_dirty();
    }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_preheat_minutes(uint16_t minutes) {
    // Clamp reasonable range 0..30
    if (minutes > 30) minutes = 30;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.preheat_min != minutes) {
        g_settings.preheat_min = minutes;
        wiseman_mark_dirty();
    }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_display_brightness(uint8_t pct) {
    if (pct > 100) {
        pct = 100;
    } 
    
    if (pct < 5) {
        pct = 5; // enforce minimum operational brightness
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.display_brightness != pct) {
        g_settings.display_brightness = pct;
        wiseman_mark_dirty();
    }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_wifi_credentials(const char* ssid, const char* pass) {
    if (!ssid) ssid = "";
    if (!pass) pass = "";
    // Ensure null termination and length limits
    char new_ssid[33] = {0};
    char new_pass[65] = {0};
    strncpy(new_ssid, ssid, sizeof(new_ssid)-1);
    strncpy(new_pass, pass, sizeof(new_pass)-1);
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (strncmp(g_settings.wifi_ssid, new_ssid, sizeof(g_settings.wifi_ssid)) != 0 ||
        strncmp(g_settings.wifi_pass, new_pass, sizeof(g_settings.wifi_pass)) != 0) {
        memcpy(g_settings.wifi_ssid, new_ssid, sizeof(g_settings.wifi_ssid));
        memcpy(g_settings.wifi_pass, new_pass, sizeof(g_settings.wifi_pass));
        wiseman_mark_dirty();
    }
    xSemaphoreGive(s_mutex);
}

uint32_t wiseman_get_op_time_minutes(void) {
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    uint32_t v = g_settings.op_time_min;
    xSemaphoreGive(s_mutex);
    return v;
}

void wiseman_add_op_time_minutes(uint32_t minutes) {
    if (minutes == 0) return;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    uint64_t sum = (uint64_t)g_settings.op_time_min + (uint64_t)minutes;
    if (sum > 0xFFFFFFFFu) sum = 0xFFFFFFFFu; // saturate
    g_settings.op_time_min = (uint32_t)sum;
    xSemaphoreGive(s_mutex);
    // Mark dirty so that updated operating time is saved. This is called at most once per minute.
    wiseman_mark_dirty();
}
