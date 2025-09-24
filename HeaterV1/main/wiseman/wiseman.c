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
static const wiseman_settings_t g_defaults = {
    .version = WISEMAN_SETTINGS_VERSION,
    .setpoint1_c = 30,
    .setpoint2_c = 30,
    .heater1_enabled = false,
    .heater2_enabled = false,
    .sound_enabled = true,
    .display_brightness = 75,
    .wifi_ssid = "",
    .wifi_pass = "",
    .op_time_min = 0,
    .sleep_timeout_s = 30,
};

// Legacy autosave timer removed; persistence handled by wiseman_persist background task.

static esp_err_t nvs_open_rw(nvs_handle_t* out) {
    nvs_handle_t h;
    esp_err_t err = nvs_open(WISEMAN_NAMESPACE, NVS_READWRITE, &h);
    if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        // Should not happen if nvs_flash_init succeeds
        return err;
    }
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Namespace created on first write; treat as success by reopening
        err = nvs_open(WISEMAN_NAMESPACE, NVS_READWRITE, &h);
    }
    if (err == ESP_OK) *out = h;
    return err;
}

static void apply_defaults(void) {
    memcpy(&g_settings, &g_defaults, sizeof(g_settings));
}

static wiseman_settings_t s_last_saved; // snapshot to avoid redundant writes
static bool s_have_snapshot = false;

static bool load_from_nvs(void) {
    nvs_handle_t h;
    if (nvs_open_rw(&h) != ESP_OK) return false;
    size_t required = 0;
    esp_err_t err = nvs_get_blob(h, WISEMAN_KEY, NULL, &required);
    if (err != ESP_OK || required == 0) { nvs_close(h); return false; }
    uint8_t *buf = (uint8_t*)malloc(required);
    if (!buf) { nvs_close(h); return false; }
    size_t len = required;
    err = nvs_get_blob(h, WISEMAN_KEY, buf, &len);
    nvs_close(h);
    if (err != ESP_OK) { free(buf); return false; }

    // Start from compiled defaults, then overlay stored bytes (migration-friendly)
    apply_defaults();
    size_t to_copy = len < sizeof(g_settings) ? len : sizeof(g_settings);
    memcpy(&g_settings, buf, to_copy);
    // Always set runtime struct version to current
    g_settings.version = WISEMAN_SETTINGS_VERSION;
    free(buf);
    return true;
}

bool wiseman_save_now(void) {
    if (!s_mutex) return false;
    // Copy current settings under lock, then perform flash I/O without holding mutex.
    wiseman_settings_t local_copy;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (s_have_snapshot && memcmp(&g_settings, &s_last_saved, sizeof(g_settings)) == 0) {
        xSemaphoreGive(s_mutex);
        return true; // nothing changed
    }
    memcpy(&local_copy, &g_settings, sizeof(local_copy));
    xSemaphoreGive(s_mutex);

    nvs_handle_t h;
    esp_err_t open_err = nvs_open_rw(&h);
    if (open_err != ESP_OK) {
        return false;
    }
    TickType_t t0 = xTaskGetTickCount();
    esp_err_t err = nvs_set_blob(h, WISEMAN_KEY, &local_copy, sizeof(local_copy));
    if (err == ESP_OK) err = nvs_commit(h);
    TickType_t t1 = xTaskGetTickCount();
    nvs_close(h);
    if (err == ESP_OK) {
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        memcpy(&s_last_saved, &local_copy, sizeof(s_last_saved));
        s_have_snapshot = true;
        xSemaphoreGive(s_mutex);
        uint32_t ms = (uint32_t)pdTICKS_TO_MS(t1 - t0);
        if (ms > 30) {
            ESP_LOGW(TAG, "NVS commit latency %u ms", (unsigned)ms);
        } else {
            ESP_LOGD(TAG, "NVS commit %u ms", (unsigned)ms);
        }
    }
    return (err == ESP_OK);
}

bool wiseman_reset_to_defaults(void) {
    apply_defaults();
#ifdef WISEMAN_DISABLE_PERSIST
    return true;
#else
    return wiseman_save_now();
#endif
}

bool wiseman_init(void) {
#ifdef WISEMAN_DISABLE_PERSIST
    ESP_LOGW(TAG, "Wiseman persistence DISABLED (test mode)");
#else
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
#endif

    if (!s_mutex) s_mutex = xSemaphoreCreateMutex();
    configASSERT(s_mutex);

    #ifdef WISEMAN_DISABLE_PERSIST
        apply_defaults();
        s_have_snapshot = false; // skip snapshot logic
    #else
        if (!load_from_nvs()) {
            ESP_LOGI(TAG, "loading defaults");
            apply_defaults();
            (void)wiseman_save_now();
        }
        else {
            // Initialize snapshot with loaded data
            memcpy(&s_last_saved, &g_settings, sizeof(g_settings));
            s_have_snapshot = true;
        }
        // Start persistence task (idempotent)
        wiseman_persist_start();
    #endif
    // No global autosave. Setpoints have their own debounce timer (except disabled in test).
    return true;
}

const wiseman_settings_t* wiseman_get(void) { return &g_settings; }

bool wiseman_get_copy(wiseman_settings_t* out) {
    if (!out) return false;
    if (!s_mutex) return false;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    memcpy(out, &g_settings, sizeof(g_settings));
    xSemaphoreGive(s_mutex);
    return true;
}

void wiseman_set_setpoint1(int16_t c) {
    if (!s_mutex) return; 
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    bool changed = (g_settings.setpoint1_c != c);
    if (changed) g_settings.setpoint1_c = c;
    xSemaphoreGive(s_mutex);
#ifndef WISEMAN_DISABLE_PERSIST
    if (changed) wiseman_mark_dirty();
#endif
}

void wiseman_set_sleep_timeout_seconds(uint16_t seconds) {
    if (!s_mutex) return;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    bool changed = (g_settings.sleep_timeout_s != seconds);
    if (changed) g_settings.sleep_timeout_s = seconds;
    xSemaphoreGive(s_mutex);
#ifndef WISEMAN_DISABLE_PERSIST
    if (changed) wiseman_mark_dirty();
#endif
}

void wiseman_set_heaters_enabled(bool ch1, bool ch2) {
    if (!s_mutex) return;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    bool changed = false;
    if (g_settings.heater1_enabled != ch1) { g_settings.heater1_enabled = ch1; changed = true; }
    if (g_settings.heater2_enabled != ch2) { g_settings.heater2_enabled = ch2; changed = true; }
    xSemaphoreGive(s_mutex);
#ifndef WISEMAN_DISABLE_PERSIST
    if (changed) wiseman_mark_dirty();
#endif
}
void wiseman_set_setpoint2(int16_t c) {
    if (!s_mutex) return; 
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    bool changed = (g_settings.setpoint2_c != c);
    if (changed) g_settings.setpoint2_c = c;
    xSemaphoreGive(s_mutex);
#ifndef WISEMAN_DISABLE_PERSIST
    if (changed) wiseman_mark_dirty();
#endif
}

void wiseman_set_dual_setpoints(int16_t sp1, int16_t sp2) {
    if (!s_mutex) return;
    bool changed = false;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.setpoint1_c != sp1) { g_settings.setpoint1_c = sp1; changed = true; }
    if (g_settings.setpoint2_c != sp2) { g_settings.setpoint2_c = sp2; changed = true; }
    xSemaphoreGive(s_mutex);
#ifndef WISEMAN_DISABLE_PERSIST
    if (changed) wiseman_mark_dirty();
#endif
}

void wiseman_set_sound_enabled(bool en) {
    if (!s_mutex) return; 
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (g_settings.sound_enabled != en) { g_settings.sound_enabled = en; }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_display_brightness(uint8_t pct) {
    if (pct > 100) pct = 100;
    if (!s_mutex) return; 
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    bool changed = (g_settings.display_brightness != pct);
    if (changed) {
        g_settings.display_brightness = pct;
    }
    xSemaphoreGive(s_mutex);
    if (changed) { wiseman_mark_dirty(); }
}

void wiseman_set_wifi_credentials(const char* ssid, const char* pass) {
    if (!ssid) ssid = "";
    if (!pass) pass = "";
    // Ensure null termination and length limits
    char new_ssid[33] = {0};
    char new_pass[65] = {0};
    strncpy(new_ssid, ssid, sizeof(new_ssid)-1);
    strncpy(new_pass, pass, sizeof(new_pass)-1);
    if (!s_mutex) return;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (strncmp(g_settings.wifi_ssid, new_ssid, sizeof(g_settings.wifi_ssid)) != 0 ||
        strncmp(g_settings.wifi_pass, new_pass, sizeof(g_settings.wifi_pass)) != 0) {
        memcpy(g_settings.wifi_ssid, new_ssid, sizeof(g_settings.wifi_ssid));
        memcpy(g_settings.wifi_pass, new_pass, sizeof(g_settings.wifi_pass));
    }
    xSemaphoreGive(s_mutex);
}

void wiseman_set_autosave_timeout(uint32_t seconds) {
    (void)seconds; // deprecated; handled by fixed debounce in persistence module
}

uint32_t wiseman_get_op_time_minutes(void) {
    if (!s_mutex) return 0;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    uint32_t v = g_settings.op_time_min;
    xSemaphoreGive(s_mutex);
    return v;
}

void wiseman_add_op_time_minutes(uint32_t minutes) {
    if (minutes == 0) return;
    if (!s_mutex) return;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    uint64_t sum = (uint64_t)g_settings.op_time_min + (uint64_t)minutes;
    if (sum > 0xFFFFFFFFu) sum = 0xFFFFFFFFu; // saturate
    g_settings.op_time_min = (uint32_t)sum;
    xSemaphoreGive(s_mutex);
}
