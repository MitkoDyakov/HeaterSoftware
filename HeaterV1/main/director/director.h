#pragma once
#include <stdbool.h>
#include "freertos/queue.h"
#include "switchboard/user_input.h" // for event_msg_t

// Start the GUI (LVGL director task). The caller provides:
//  - button_event_queue: events from inputdetect
//  - sample_queue: single-slot queue (length 1) receiving latest fireman_sample_t via overwrite
bool director_start(QueueHandle_t button_event_queue, QueueHandle_t sample_queue);
#ifndef LV_DIRECTOR_H
#define LV_DIRECTOR_H

#endif /* LV_DIRECTOR_H */  