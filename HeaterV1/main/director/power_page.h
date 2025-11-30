#ifndef POWER_PAGE_H
#define POWER_PAGE_H

#include "lvgl.h"
#include "switchboard.h"  // for event_msg_t
#include "mailman/ap33772s.h"  // for ap33772s_caps_t

void page_power_create(lv_obj_t *page_container, const ap33772s_caps_t *pd_caps);
void power_page_handle_event(event_msg_t msg);
void page_power_cleanup(void);

#endif // POWER_PAGE_H
