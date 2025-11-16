// SPDX-License-Identifier: MIT
#ifndef SWITCHBOARD_H
#define SWITCHBOARD_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_err.h"

// ---------- Events ----------
typedef enum {
    BUTTON_EVENT_SHORT,
    BUTTON_EVENT_REPEAT
} button_event_t;

typedef struct {
    int            btn_id;   // GPIO number (or your own ID)
    button_event_t event;
} event_msg_t;

// Setup now requires an externally created queue (item size = sizeof(event_msg_t)).
// Returns ESP_OK on success, ESP_ERR_INVALID_ARG if queue is NULL,
// or other ESP error codes on GPIO/timer setup failure.
esp_err_t switchboard_init(QueueHandle_t event_queue);

#endif // SWITCHBOARD_H
