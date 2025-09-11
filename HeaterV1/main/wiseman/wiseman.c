#include "wiseman.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <string.h>
#include <stdlib.h>

#define WISEMAN_NAMESPACE "wiseman"
#define WISEMAN_KEY       "settings"
static const char* TAG = "WISEMAN";

// In-RAM settings and defaults
static wiseman_settings_t g_settings;
static const wiseman_settings_t g_defaults = {
    .version = WISEMAN_SETTINGS_VERSION,
    .setpoint1_c = 0,
    .setpoint2_c = 0,
    .heater1_enabled = false,
    .heater2_enabled = false,
    .sound_enabled = true,
    .display_brightness = 50,
    .wifi_ssid = "",
    .wifi_pass = "",
};

// Autosave debounce state
static TimerHandle_t s_autosave_timer = NULL;
static uint32_t s_autosave_secs = WISEMAN_AUTOSAVE_SECS_DEFAULT;
static bool s_dirty = false;

static void start_autosave_timer(void);
static void autosave_cb(TimerHandle_t xTimer) {
    (void)xTimer;
    if (s_dirty) {
        if (wiseman_save_now()) {
            s_dirty = false;
            ESP_LOGI(TAG, "autosave complete");
        } else {
            ESP_LOGW(TAG, "autosave failed; will retry later");
            // Keep dirty to attempt next change or next timer run
        }
    }
}

static void mark_dirty(void) {
    s_dirty = true;
    start_autosave_timer();
}

static void start_autosave_timer(void) {
    if (s_autosave_secs == 0) return; // disabled
    if (!s_autosave_timer) {
        s_autosave_timer = xTimerCreate("wis_autosave", pdMS_TO_TICKS(1000 * s_autosave_secs), pdFALSE, NULL, autosave_cb);
    }
    if (s_autosave_timer) {
        xTimerStop(s_autosave_timer, 0);
        xTimerChangePeriod(s_autosave_timer, pdMS_TO_TICKS(1000 * s_autosave_secs), 0);
        xTimerStart(s_autosave_timer, 0);
    }
}

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
    nvs_handle_t h;
    if (nvs_open_rw(&h) != ESP_OK) return false;
    esp_err_t err = nvs_set_blob(h, WISEMAN_KEY, &g_settings, sizeof(g_settings));
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return (err == ESP_OK);
}

bool wiseman_reset_to_defaults(void) {
    apply_defaults();
    return wiseman_save_now();
}

bool wiseman_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    if (!load_from_nvs()) {
        ESP_LOGI(TAG, "loading defaults");
        apply_defaults();
        (void)wiseman_save_now();
    }
    s_dirty = false;
    start_autosave_timer();
    return true;
}

const wiseman_settings_t* wiseman_get(void) { return &g_settings; }

void wiseman_set_setpoint1(int16_t c) {
    if (g_settings.setpoint1_c != c) { g_settings.setpoint1_c = c; mark_dirty(); }
}
void wiseman_set_setpoint2(int16_t c) {
    if (g_settings.setpoint2_c != c) { g_settings.setpoint2_c = c; mark_dirty(); }
}

void wiseman_set_sound_enabled(bool en) {
    if (g_settings.sound_enabled != en) { g_settings.sound_enabled = en; mark_dirty(); }
}

void wiseman_set_display_brightness(uint8_t pct) {
    if (pct > 100) pct = 100;
    if (g_settings.display_brightness != pct) { g_settings.display_brightness = pct; mark_dirty(); }
}

void wiseman_set_wifi_credentials(const char* ssid, const char* pass) {
    if (!ssid) ssid = "";
    if (!pass) pass = "";
    // Ensure null termination and length limits
    char new_ssid[33] = {0};
    char new_pass[65] = {0};
    strncpy(new_ssid, ssid, sizeof(new_ssid)-1);
    strncpy(new_pass, pass, sizeof(new_pass)-1);
    if (strncmp(g_settings.wifi_ssid, new_ssid, sizeof(g_settings.wifi_ssid)) != 0 ||
        strncmp(g_settings.wifi_pass, new_pass, sizeof(g_settings.wifi_pass)) != 0) {
        memcpy(g_settings.wifi_ssid, new_ssid, sizeof(g_settings.wifi_ssid));
        memcpy(g_settings.wifi_pass, new_pass, sizeof(g_settings.wifi_pass));
        mark_dirty();
    }
}

void wiseman_set_autosave_timeout(uint32_t seconds) {
    s_autosave_secs = seconds;
    start_autosave_timer();
}
