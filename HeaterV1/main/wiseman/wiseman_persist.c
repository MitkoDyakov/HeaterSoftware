#include "wiseman_persist.h"
#include "wiseman.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#ifdef WISEMAN_DISABLE_PERSIST

bool wiseman_persist_start(void) { return true; }
void wiseman_mark_dirty(void) {}
void wiseman_request_flush(void) {}

#else

#define WISEMAN_PERSIST_TASK_STACK 4096
#define WISEMAN_PERSIST_TASK_PRIO  2   // lower priority to reduce UI interference
#define WISEMAN_DEBOUNCE_MS        10000  // slightly longer debounce to batch changes
#define WISEMAN_POLL_INTERVAL_MS   100   // task wake period

static const char *TAG = "WISEMAN_PERSIST";
static TaskHandle_t s_task = NULL;
static SemaphoreHandle_t s_flag_mutex = NULL;
static bool s_dirty = false;
static bool s_force_flush = false;
static TickType_t s_last_mark_tick = 0;

// Statistics
static uint32_t s_saves_attempted = 0;
static uint32_t s_saves_ok = 0;
static uint32_t s_last_latency_ms = 0;
static uint32_t s_max_latency_ms = 0;
static uint32_t s_forced_flushes = 0;
static uint32_t s_debounced_marks = 0;
static uint32_t s_task_stack_low_water = 0; // words remaining

static void persist_task(void *arg) {
    (void)arg;
    for(;;) {
        bool do_save = false;
        xSemaphoreTake(s_flag_mutex, portMAX_DELAY);
        if (s_force_flush) {
            do_save = true;
            s_force_flush = false;
        } else if (s_dirty) {
            TickType_t now = xTaskGetTickCount();
            if ((now - s_last_mark_tick) >= pdMS_TO_TICKS(WISEMAN_DEBOUNCE_MS)) {
                do_save = true;
            }
        }
        xSemaphoreGive(s_flag_mutex);

        if (do_save) {
            s_saves_attempted++;
            TickType_t t0 = xTaskGetTickCount();
            bool ok = wiseman_save_now();
            TickType_t t1 = xTaskGetTickCount();
            uint32_t ms = (uint32_t)pdTICKS_TO_MS(t1 - t0);
            s_last_latency_ms = ms;
            if (ms > s_max_latency_ms) s_max_latency_ms = ms;
            if (ok) {
                s_saves_ok++;
                ESP_LOGD(TAG, "settings saved (%u ms)", (unsigned)ms);
                xSemaphoreTake(s_flag_mutex, portMAX_DELAY);
                s_dirty = false;
                xSemaphoreGive(s_flag_mutex);
            } else {
                ESP_LOGW(TAG, "save failed (%u ms); will retry", (unsigned)ms);
                // leave dirty flag set for retry
            }
            // Stack watermark sampling
            UBaseType_t hw = uxTaskGetStackHighWaterMark(NULL); // words remaining
            if (hw < s_task_stack_low_water || s_task_stack_low_water == 0) {
                s_task_stack_low_water = hw;
                if (hw < 256) {
                    ESP_LOGW(TAG, "low stack watermark: %u words", (unsigned)hw);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(WISEMAN_POLL_INTERVAL_MS));
    }
}

#endif /* WISEMAN_DISABLE_PERSIST */

#ifndef WISEMAN_DISABLE_PERSIST
bool wiseman_persist_start(void) {
    if (s_task) return true; // already started
    if (!s_flag_mutex) {
        s_flag_mutex = xSemaphoreCreateMutex();
        if (!s_flag_mutex) return false;
    }
    if (xTaskCreate(persist_task, "wiseman_persist", WISEMAN_PERSIST_TASK_STACK, NULL, WISEMAN_PERSIST_TASK_PRIO, &s_task) != pdPASS) {
        ESP_LOGE(TAG, "failed to create persist task");
        return false;
    }
    return true;
}

void wiseman_mark_dirty(void) {
    if (!s_flag_mutex) return; // not started yet
    xSemaphoreTake(s_flag_mutex, portMAX_DELAY);
    if (!s_dirty) {
        s_dirty = true;
        s_last_mark_tick = xTaskGetTickCount();
    } else {
        // already dirty; just refresh timestamp to extend debounce
        s_last_mark_tick = xTaskGetTickCount();
        s_debounced_marks++;
    }
    xSemaphoreGive(s_flag_mutex);
}

void wiseman_request_flush(void) {
    if (!s_flag_mutex) return; // not started yet
    xSemaphoreTake(s_flag_mutex, portMAX_DELAY);
    s_force_flush = true;
    s_forced_flushes++;
    xSemaphoreGive(s_flag_mutex);
}

void wiseman_persist_get_stats(wiseman_persist_stats_t* out) {
    if (!out) return;
    if (!s_flag_mutex) {
        // zeroed if not started
        *out = (wiseman_persist_stats_t){0};
        return;
    }
    xSemaphoreTake(s_flag_mutex, portMAX_DELAY);
    wiseman_persist_stats_t snap = {
        .saves_attempted = s_saves_attempted,
        .saves_ok = s_saves_ok,
        .last_latency_ms = s_last_latency_ms,
        .max_latency_ms = s_max_latency_ms,
        .forced_flushes = s_forced_flushes,
        .debounced_marks = s_debounced_marks,
        .task_stack_low_water = s_task_stack_low_water,
    };
    xSemaphoreGive(s_flag_mutex);
    *out = snap;
}
#endif /* !WISEMAN_DISABLE_PERSIST */
