#include "wiseman_persist.h"
#include "wiseman.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define WISEMAN_PERSIST_TASK_STACK 4096
#define WISEMAN_PERSIST_TASK_PRIO  2      // lower priority to reduce UI interference
#define WISEMAN_DEBOUNCE_MS        10000  // slightly longer debounce to batch changes
#define WISEMAN_POLL_INTERVAL_MS   100    // task wake period

static const char *TAG = "WISEMAN_PERSIST";
static TaskHandle_t s_task = NULL;
static SemaphoreHandle_t s_flag_mutex = NULL;
static bool s_dirty = false;
static bool s_force_flush = false;
static TickType_t s_last_mark_tick = 0;

static void persist_task(void *arg) {
    (void)arg;

    for(;;) {
        bool do_save = false;

        xSemaphoreTake(s_flag_mutex, portMAX_DELAY);        
        if (s_force_flush) {
            do_save = true;
            s_force_flush = false;
            s_dirty = false;  // Clear dirty flag when force flushing
        } else if (s_dirty) {
            TickType_t now = xTaskGetTickCount();
            if ((now - s_last_mark_tick) >= pdMS_TO_TICKS(WISEMAN_DEBOUNCE_MS)) {
                do_save = true;
            }
        }
        xSemaphoreGive(s_flag_mutex);

        if (do_save) {
            bool ok = wiseman_save_now();
            if (ok) {
                ESP_LOGD(TAG, "settings saved");
                xSemaphoreTake(s_flag_mutex, portMAX_DELAY);
                s_dirty = false;
                xSemaphoreGive(s_flag_mutex);
            } else {
                ESP_LOGW(TAG, "save failed; will retry");
                // leave dirty flag set for retry
            }

        }

        vTaskDelay(pdMS_TO_TICKS(WISEMAN_POLL_INTERVAL_MS));
    }
}

bool wiseman_persist_start(void) {
    if (s_task != NULL) {
        return true; // already started
    } 

    if (s_flag_mutex == NULL) {
        s_flag_mutex = xSemaphoreCreateMutex();        
    }

    if (s_flag_mutex == NULL) {
        return false;       
    }

    if (xTaskCreate(persist_task, "wiseman_persist", WISEMAN_PERSIST_TASK_STACK, NULL, WISEMAN_PERSIST_TASK_PRIO, &s_task) != pdPASS) {
        ESP_LOGE(TAG, "failed to create persist task");
        return false;
    }

    return true;
}

void wiseman_mark_dirty(void) {
    xSemaphoreTake(s_flag_mutex, portMAX_DELAY);
    if (!s_dirty) {
        s_dirty = true;
        s_last_mark_tick = xTaskGetTickCount();
    } else {
        // already dirty; just refresh timestamp to extend debounce
        s_last_mark_tick = xTaskGetTickCount();
    }
    xSemaphoreGive(s_flag_mutex);
}

void wiseman_request_flush(void) {
    xSemaphoreTake(s_flag_mutex, portMAX_DELAY);
    s_force_flush = true;
    xSemaphoreGive(s_flag_mutex);
}

