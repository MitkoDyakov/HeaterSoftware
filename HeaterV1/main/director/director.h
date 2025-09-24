#pragma once
#include <stdbool.h>
#include "freertos/queue.h"
#include "switchboard/user_input.h" // for event_msg_t
#include "AP33772S.h" // use single definition of ap33772s_caps_t

bool director_start(QueueHandle_t button_event_queue,
                    QueueHandle_t sample_queue,
                    const ap33772s_caps_t *initial_pd_caps);