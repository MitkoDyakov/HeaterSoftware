#include "settings_page.h"
#include "ui.h"
#include "wiseman/wiseman.h"
#include "wiseman/wiseman_persist.h"
#include "director/backlight.h"
#include <string.h>
#include "pinout.h"
// Window/scroll constants
#define WINDOW_COUNT 5
#define ROW_HEIGHT_PX 23

// State
static bool s_settings_edit_mode = false;      // true while adjusting a setting
static bool s_settings_blink_on = false;       // blink phase for selection indicator
static int  s_settings_selected_idx = 0;       // persistent selection index (0..SETTINGS_COUNT-1)
static lv_obj_t * s_page_container = NULL;     // page container passed on enter
static lv_obj_t * s_scroll_column = NULL;      // cached pointer to scrollColumn for padding updates
static int window_start_idx = 0;               // first visible setting index in window
static int window_end_idx = WINDOW_COUNT - 1;  // last visible setting index in window
static int s_last_scroll_pad_top = 0;          // last applied pad_top to avoid redundant layout work

// Local helper to find object by name (duplicated from director for isolation)
static lv_obj_t * find_obj_by_name(lv_obj_t * root, const char * name) {
    if (!root || !name) return NULL;
    const char * n = lv_obj_get_name(root);
    if (n && strcmp(n, name) == 0) return root;
    uint32_t child_cnt = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t * child = lv_obj_get_child(root, i);
        lv_obj_t * found = find_obj_by_name(child, name);
        if (found) return found;
    }
    return NULL;
}

// Maintain a sliding window and shift content vertically via pad_top so selected row stays visible.
static void settings_scroll_sync(void) {
    if (!s_scroll_column) {
        s_scroll_column = find_obj_by_name(s_page_container, "scrollColumn");
        if (!s_scroll_column) return;
    }
    int sel = s_settings_selected_idx;
    if (sel < 0) sel = 0; else if (sel >= SETTINGS_COUNT) sel = SETTINGS_COUNT - 1;
    if (sel < window_start_idx) {
        window_start_idx = sel;
    }
    if (sel > window_end_idx) {
        window_start_idx = sel - (WINDOW_COUNT - 1);
        if (window_start_idx < 0) window_start_idx = 0;
    }
    int max_start = SETTINGS_COUNT - WINDOW_COUNT;
    if (max_start < 0) max_start = 0;
    if (window_start_idx > max_start) window_start_idx = max_start;
    window_end_idx = window_start_idx + WINDOW_COUNT - 1;
    if (window_end_idx >= SETTINGS_COUNT) window_end_idx = SETTINGS_COUNT - 1;

    int pad = -(window_start_idx * ROW_HEIGHT_PX);
    if (pad != s_last_scroll_pad_top) {
        s_last_scroll_pad_top = pad;
        lv_obj_set_style_pad_top(s_scroll_column, pad, 0);
        lv_obj_mark_layout_as_dirty(s_scroll_column);
    }
}

// -------- Settings page helpers --------
static void settings_cycle_enable(int dir) {
    const char* cur = lv_subject_get_string(&activeCh);
    int idx;
    if (!cur) idx = 0;
    else if (strcmp(cur, "CH1") == 0) idx = 1;
    else if (strcmp(cur, "CH2") == 0) idx = 2;
    else idx = 0; // "CH1/2"
    idx = (idx + (dir > 0 ? 1 : -1) + 3) % 3;
    switch(idx){
        case 0: lv_subject_copy_string(&activeCh, "CH1/2"); break;
        case 1: lv_subject_copy_string(&activeCh, "CH1");   break;
        case 2: lv_subject_copy_string(&activeCh, "CH2");   break;
    }
}

static void settings_toggle_sound(void) {
    const char* cur = lv_subject_get_string(&soundEnable);
    if (cur && strcmp(cur, "ON") == 0) lv_subject_copy_string(&soundEnable, "OFF");
    else                                 lv_subject_copy_string(&soundEnable, "ON");
}

// Adjust the value for the currently edited row. The XML row order is:
// 0 TEMP, 1 PREHEAT, 2 TIMER, 3 SOUND, 4 LIGHT(brightness), 5 SLEEP, 6 FLIP.
// Only rows with existing subjects mapped to adjustable settings are handled for now.
static void settings_adjust_value(int activeSetting, int dir) {
    switch (activeSetting) {
        case 0: { // TEMP
            int t = lv_subject_get_int(&default_temp);
            t += (dir > 0 ? 1 : -1);
            if (t < 0) t = 0; else if (t > 60) t = 60;
            lv_subject_set_int(&default_temp, t);
        } break;
        case 3: { // SOUND
            if (dir != 0) settings_toggle_sound();
        } break;
        case 4: { // LIGHT (DISPLAY BRIGHTNESS % step 5)
            int b = lv_subject_get_int(&brightness);
            b += (dir > 0 ? 5 : -5);
            if (b < 5) b = 5; else if (b > 100) b = 100;
            lv_subject_set_int(&brightness, b);
            backlight_set_brightness((uint8_t)b);
        } break;
        case 5: { // SLEEP timer (s) step 5, 0=OFF
            int s = lv_subject_get_int(&sleepTimer);
            s += (dir > 0 ? 5 : -5);
            if (s < 0) s = 0; else if (s > 60) s = 60;
            lv_subject_set_int(&sleepTimer, s);
        } break;
        // TODO: implement PREHEAT (1), TIMER (2), FLIP (6) when behavior decided.
        default: break;
    }
}

void settings_page_blink_cb(lv_timer_t *t) {
    (void)t;
    if (s_settings_edit_mode) {
        s_settings_blink_on = !s_settings_blink_on;
        int idx = s_settings_selected_idx;
        if (idx < 0) idx = 0; else if (idx >= SETTINGS_COUNT) idx = SETTINGS_COUNT - 1;
        lv_subject_set_int(&settingsSelect, s_settings_blink_on ? -1 : idx);
    } else {
        int cur = lv_subject_get_int(&settingsSelect);
        if (cur != s_settings_selected_idx) {
            lv_subject_set_int(&settingsSelect, s_settings_selected_idx);
        }
    }
}

void settings_page_enter(lv_obj_t *page_container) {
    s_page_container = page_container;
    (void)settings_create(s_page_container);
    s_settings_edit_mode = false;
    s_settings_blink_on = false;
    s_settings_selected_idx = 0;
    lv_subject_set_int(&settingsSelect, s_settings_selected_idx);
    window_start_idx = 0;
    window_end_idx = WINDOW_COUNT - 1;
    s_scroll_column = find_obj_by_name(s_page_container, "scrollColumn");
    if (s_scroll_column) {
        s_last_scroll_pad_top = 0;
        lv_obj_set_style_pad_top(s_scroll_column, 0, 0);
        lv_obj_mark_layout_as_dirty(s_scroll_column);
    }
}

void settings_page_handle_event(event_msg_t msg) {
    int activeSetting = s_settings_edit_mode ? s_settings_selected_idx
                                             : lv_subject_get_int(&settingsSelect);
    if (!s_settings_edit_mode) {
        if (activeSetting < 0) activeSetting = 0; else if (activeSetting >= SETTINGS_COUNT) activeSetting = SETTINGS_COUNT - 1;
        s_settings_selected_idx = activeSetting;
    }

    if (msg.event == BUTTON_EVENT_SHORT) {
        switch (msg.btn_id) {
            case BUTTON_RIGHT_TOP: { // UP
                if (s_settings_edit_mode) {
                    settings_adjust_value(activeSetting, +1);
                } else if (activeSetting > 0) {
                    int idx = activeSetting - 1;
                    s_settings_selected_idx = idx;
                    lv_subject_set_int(&settingsSelect, idx);
                    settings_scroll_sync();
                }
            } break;
            case BUTTON_RIGHT_BOTTOM: { // DOWN
                if (s_settings_edit_mode) {
                    settings_adjust_value(activeSetting, -1);
                } else if (activeSetting < SETTINGS_COUNT - 1) {
                    int idx = activeSetting + 1;
                    s_settings_selected_idx = idx;
                    lv_subject_set_int(&settingsSelect, idx);
                    settings_scroll_sync();
                }
            } break;
            case BUTTON_RIGHT_CENTER: { // ENTER/CONFIRM
                if (!s_settings_edit_mode) {
                    int cur = lv_subject_get_int(&settingsSelect);
                    if (cur < 0) cur = s_settings_selected_idx;
                    if (cur < 0) cur = 0; else if (cur >= SETTINGS_COUNT) cur = SETTINGS_COUNT - 1;
                    s_settings_selected_idx = cur;
                    s_settings_blink_on = false;
                    lv_subject_set_int(&settingsSelect, s_settings_selected_idx);
                    s_settings_edit_mode = true;
                    settings_scroll_sync();
                } else {
                    s_settings_edit_mode = false;
                    s_settings_blink_on = false;
                    lv_subject_set_int(&settingsSelect, s_settings_selected_idx);
                    settings_scroll_sync();
                }
            } break;
            default: break;
        }
    } else if (msg.event == BUTTON_EVENT_REPEAT) {
        if (s_settings_edit_mode) {
            switch (msg.btn_id) {
                case BUTTON_RIGHT_TOP:
                    settings_adjust_value(activeSetting, +1);
                    break;
                case BUTTON_RIGHT_BOTTOM:
                    settings_adjust_value(activeSetting, -1);
                    break;
                default:
                    break;
            }
        }
    }
}

void settings_page_leave(void) {
    // Persist settings on leaving Settings page
    s_settings_edit_mode = false;
    s_settings_blink_on = false;
    if (lv_subject_get_int(&settingsSelect) != s_settings_selected_idx) {
        lv_subject_set_int(&settingsSelect, s_settings_selected_idx);
    }

    const char* ch = lv_subject_get_string(&activeCh);
    bool ch1 = false;
    bool ch2 = false;
    if (ch) {
        if (strcmp(ch, "CH1") == 0) { ch1 = true; ch2 = false; }
        else if (strcmp(ch, "CH2") == 0) { ch1 = false; ch2 = true; }
        else { ch1 = true; ch2 = true; }
    }

    int t = lv_subject_get_int(&default_temp);
    if (t < 0) t = 0; else if (t > 60) t = 60;
    wiseman_set_setpoint1((int16_t)t);

    int b = lv_subject_get_int(&brightness);
    if (b < 0) b = 0; else if (b > 100) b = 100;
    wiseman_set_display_brightness((uint8_t)b);

    int st = lv_subject_get_int(&sleepTimer);
    if (st < 0) st = 0; else if (st > 60) st = 60;
    wiseman_set_sleep_timeout_seconds((uint16_t)st);

    const char* snd = lv_subject_get_string(&soundEnable);
    wiseman_set_sound_enabled(snd && strcmp(snd, "ON") == 0);
    wiseman_set_heaters_enabled(ch1, ch2);

    wiseman_request_flush();
}
