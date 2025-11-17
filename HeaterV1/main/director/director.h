#ifndef DIRECTOR_H
#define DIRECTOR_H

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "switchboard/switchboard.h"  // for event_msg_t
#include "mailman/ap33772s.h"         // for ap33772s_caps_t

/**
 * @brief Initialize the director (GUI and display controller)
 * 
 * The director manages the user interface, display, and coordinates between
 * user input and system modules. It handles button events, temperature display,
 * and PD power management UI.
 * 
 * @param button_event_queue Queue for receiving button events from switchboard
 * @param temperature_queue Queue for receiving temperature samples from fireman
 * @param i2c_queue Queue for sending I2C requests to mailman
 * @param initial_pd_caps Initial PD capabilities discovered during startup
 * @return true if initialization succeeded, false otherwise
 */

bool director_init(QueueHandle_t button_event_queue, QueueHandle_t temperature_queue, QueueHandle_t i2c_queue, const ap33772s_caps_t *initial_pd_caps);

#endif // DIRECTOR_H