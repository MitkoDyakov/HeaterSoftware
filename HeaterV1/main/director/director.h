#pragma once
#include <stdbool.h>
#include "freertos/queue.h"
#include "switchboard/user_input.h" // for event_msg_t

// Start the GUI (LVGL director task). The caller provides the button event queue
// created by inputdetect_setup. Returns false on failure (e.g. allocation).
bool director_start(QueueHandle_t button_event_queue);
#ifndef LV_DIRECTOR_H
#define LV_DIRECTOR_H

#endif /* LV_DIRECTOR_H */  