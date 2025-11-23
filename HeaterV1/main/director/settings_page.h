#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include "lvgl.h"
#include "switchboard.h"  // for event_msg_t

// Total number of configurable settings shown on the Settings page.
// Increase this when adding new settings; navigation/clamping logic uses it.
#define SETTINGS_COUNT 7

void settings_page_enter(lv_obj_t *page_container);
void settings_page_handle_event(event_msg_t msg);
void settings_page_leave(void); // persist + cleanup when leaving page
void settings_page_blink_cb(lv_timer_t *t);

#endif // SETTINGS_PAGE_H
