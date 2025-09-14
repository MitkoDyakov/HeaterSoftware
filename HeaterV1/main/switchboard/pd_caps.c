#include "pd_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static switchboard_pd_caps_t s_caps;
static bool s_caps_valid = false;
static SemaphoreHandle_t s_mutex = NULL;

static void ensure_mutex(void) {
    if (!s_mutex) {
        s_mutex = xSemaphoreCreateMutex();
    }
}

void switchboard_update_pd_caps(const switchboard_pd_caps_t *caps) {
    if (!caps) return;
    ensure_mutex();
    if (s_mutex && xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        s_caps = *caps;
        s_caps_valid = true;
        xSemaphoreGive(s_mutex);
    }
}

bool switchboard_get_pd_caps(switchboard_pd_caps_t *out) {
    if (!out) return false;
    ensure_mutex();
    bool ok = false;
    if (s_mutex && xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        if (s_caps_valid) {
            *out = s_caps;
            ok = true;
        }
        xSemaphoreGive(s_mutex);
    }
    return ok;
}
