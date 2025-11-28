#ifndef MAIN_PAGE_H
#define MAIN_PAGE_H

#include "lvgl.h"
#include "switchboard/switchboard.h"
#include "AP33772S.h"

/**
 * @brief Create the main page UI elements
 * 
 * @param container Parent container to add elements to
 * @param initial_pd_caps Pointer to initial PD capabilities (used for voltage selection)
 */
void page_main_create(lv_obj_t *container, const ap33772s_caps_t *initial_pd_caps);

/**
 * @brief Handle button events for the main page
 * 
 * Handles temperature adjustment, heater start/stop, timer edit mode
 * 
 * @param msg Button event message
 */
void main_page_handle_event(event_msg_t msg);

/**
 * @brief Clean up main page resources
 * 
 * Called when leaving the main page
 */
void page_main_cleanup(void);

#endif // MAIN_PAGE_H
