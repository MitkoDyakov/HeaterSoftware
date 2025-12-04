#include "settings_page.h"
#include "ui.h"
#include "wiseman/wiseman.h"
#include "wiseman/wiseman_persist.h"
#include "director/backlight.h"
#include <string.h>
#include "pinout.h"
#include "timekeeper.h"

// pointers to page elements
static lv_obj_t * s_page_container = NULL;     // page container passed on enter
static lv_obj_t * s_scroll_column = NULL;      // cached pointer to scrollColumn for padding updates

// variables for edit mode and selection
static bool s_settings_edit_mode = false;      // true while adjusting a setting
static bool s_settings_blink_on = false;       // blink phase for selection indicator
static int  s_settings_selected_idx = 0;       // persistent selection index (0..SETTINGS_COUNT-1)

// Constants and variables for scrolling behavior
#define WINDOW_COUNT 5
#define ROW_HEIGHT_PX 23
static int window_start_idx = 0;               // first visible setting index in window
static int s_last_scroll_pad_top = 0;          // last applied pad_top to avoid redundant layout work

// Local helper to find object by name (duplicated from director for isolation)
lv_obj_t * find_obj_by_name(lv_obj_t * root, const char * name);

// Blink timer implemented via LVGL (safer: LVGL API calls stay in LVGL task context)
static lv_timer_t *settings_blink_timer = NULL;

// Maintain a sliding window and shift content vertically via pad_top so selected row stays visible.
static void settings_scroll_sync(void) {
    if (!s_scroll_column) {
        s_scroll_column = find_obj_by_name(s_page_container, "scrollColumn");
        if (!s_scroll_column) return;
    }
    int sel = s_settings_selected_idx;
    if (sel < 0) sel = 0; else if (sel >= SETTINGS_COUNT) sel = SETTINGS_COUNT - 1;
    
    int window_end_idx = window_start_idx + WINDOW_COUNT - 1;
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
static void settings_toggle_sound(void) {
    const char* cur = lv_subject_get_string(&soundEnable);
    if (cur && strcmp(cur, "ON") == 0) 
        lv_subject_copy_string(&soundEnable, "OFF");
    else 
        lv_subject_copy_string(&soundEnable, "ON");
}

static void settings_cycle_flip(void) {
    const char* cur = lv_subject_get_string(&orientation);
    
    // OFF -> ON -> AUTO -> OFF
    if (!cur || strcmp(cur, "OFF") == 0) {
        lv_subject_copy_string(&orientation, "ON");
    } else if (strcmp(cur, "ON") == 0) {
        lv_subject_copy_string(&orientation, "AUTO");
    } else { // AUTO or unknown
        lv_subject_copy_string(&orientation, "OFF");
    }
}

static void settings_toggle_timer(void) {
    const char* cur = lv_subject_get_string(&timerType);
    bool was_on = (cur && strcmp(cur, "ON") == 0);
    bool now_on = !was_on;
    lv_subject_copy_string(&timerType, now_on ? "ON" : "OFF");
    if (was_on && !now_on) {
        // Switched from TIMER mode to STOPWATCH -> clean timekeeper display/state
        timekeeper_refresh_gui();
    }
}

// Adjust the value for the currently edited row. The XML row order is:
// 0 TEMP, 1 PREHEAT, 2 TIMER, 3 SOUND, 4 LIGHT(brightness), 5 SLEEP, 6 FLIP.
// Only rows with existing subjects mapped to adjustable settings are handled for now.
static void settings_adjust_value(int activeSetting, int dir) {
    switch (activeSetting) {
        case 0: { // TEMP
            int t = lv_subject_get_int(&default_temp);
            t = t + dir;
            if (t < 0) t = 0; 
            if (t > 60) t = 60;
            lv_subject_set_int(&default_temp, t);
        } break;
        case 1: { // PREHEAT minutes (step 5) max 30
            int p = lv_subject_get_int(&preHeat);
            p = p + (dir * 5);
            if (p < 0) p = 0; 
            if (p > 30) p = 30;
            lv_subject_set_int(&preHeat, p);
        } break;
        case 2: { // TIMER toggle OFF/ON
            settings_toggle_timer();
        } break;
        case 3: { // SOUND
            settings_toggle_sound();
        } break;
        case 4: { // LIGHT (DISPLAY BRIGHTNESS % step 5)
            int b = lv_subject_get_int(&brightness);
            b = b + (dir * 5);
            if (b < 5) b = 5;
            if (b > 100) b = 100;
            lv_subject_set_int(&brightness, b);
            backlight_set_brightness((uint8_t)b);
        } break;
        case 5: { // SLEEP timer (s) step 5, 0=OFF
            int s = lv_subject_get_int(&sleepTimer);
            s = s + (dir * 5);
            if (s < 0) s = 0; 
            if (s > 60) s = 60;
            lv_subject_set_int(&sleepTimer, s);
        } break;
        case 6: { // FLIP orientation cycle (OFF -> ON -> AUTO -> OFF)
            settings_cycle_flip();
        } break;
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

void page_settings_create(lv_obj_t *page_container) {
    // Create or resume LVGL blink timer (500 ms)
    if (settings_blink_timer == NULL) {
        settings_blink_timer = lv_timer_create(settings_page_blink_cb, 500, NULL);
    } else {
        lv_timer_resume(settings_blink_timer);
    }

    s_page_container = page_container;
    (void)settings_create(s_page_container);
    s_settings_edit_mode = false;
    s_settings_blink_on = false;
    s_settings_selected_idx = 0;
    lv_subject_set_int(&settingsSelect, s_settings_selected_idx);
    window_start_idx = 0;
    s_scroll_column = find_obj_by_name(s_page_container, "scrollColumn");
    if (s_scroll_column) {
        s_last_scroll_pad_top = 0;
        lv_obj_set_style_pad_top(s_scroll_column, 0, 0);
        lv_obj_mark_layout_as_dirty(s_scroll_column);
    }
}

void settings_page_handle_event(event_msg_t msg) {
    int activeSetting = s_settings_edit_mode ? s_settings_selected_idx : lv_subject_get_int(&settingsSelect);
    
    if (!s_settings_edit_mode) {
        if (activeSetting < 0) activeSetting = 0; 
        if (activeSetting >= SETTINGS_COUNT) activeSetting = SETTINGS_COUNT - 1;
        s_settings_selected_idx = activeSetting;
    }

    if (msg.event == BUTTON_EVENT_SHORT) {
        switch (msg.btn_id) {
            case BUTTON_RIGHT_TOP: { // UP
                if (s_settings_edit_mode) {
                    settings_adjust_value(activeSetting, +1);
                } else if (activeSetting > 0) {
                    int idx = activeSetting - 1;
                    // Skip TIMER(2) and PREHEAT(1) if heater is running
                    int running = lv_subject_get_int(&heaterRunning);
                    if (running && idx == 2) idx = 0; // from SOUND(3) -> skip to DEFAULT_TEMP(0)
                    else if (running && idx == 1) idx = 0; // from TIMER(2) edge case -> DEFAULT_TEMP(0)
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
                    // Skip PREHEAT(1) and TIMER(2) if heater is running
                    int running = lv_subject_get_int(&heaterRunning);
                    if (running && idx == 1) idx = 3; // from DEFAULT_TEMP(0) -> skip to SOUND(3)
                    else if (running && idx == 2) idx = 3; // from PREHEAT(1) edge case -> SOUND(3)
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
                        // Direct toggle rows: TIMER(2), SOUND(3), FLIP(6)
                        if (cur == 2 || cur == 3 || cur == 6) {
                            // Use unified path: adjust_value with dir=0 for toggle/cycle
                            settings_adjust_value(cur, 0);
                        } else {
                            // Enter edit mode for non-boolean rows
                            s_settings_selected_idx = cur;
                            s_settings_blink_on = false;
                            lv_subject_set_int(&settingsSelect, s_settings_selected_idx);
                            s_settings_edit_mode = true;
                            settings_scroll_sync();
                            break;
                        }
                        // For direct toggles, refresh selection highlight (no edit mode).
                        // This reasserts the selection even if blink left settingsSelect at -1,
                        // and ensures the selected row stays visible via settings_scroll_sync().
                        s_settings_selected_idx = cur;
                        lv_subject_set_int(&settingsSelect, s_settings_selected_idx);
                        settings_scroll_sync();
                    } else {
                        // Exiting edit mode
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

void page_settings_cleanup(void) {
    // Persist settings on leaving Settings page
    s_settings_edit_mode = false;
    s_settings_blink_on = false;
    if (lv_subject_get_int(&settingsSelect) != s_settings_selected_idx) {
        lv_subject_set_int(&settingsSelect, s_settings_selected_idx);
    }

    // Pause blink timer when leaving settings page (kept for lifetime)
    if (settings_blink_timer) {
        lv_timer_pause(settings_blink_timer);
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

    // Persist PREHEAT and TIMER mode
    int pre = lv_subject_get_int(&preHeat);
    if (pre < 0) pre = 0; else if (pre > 30) pre = 30;
    wiseman_set_preheat_minutes((uint16_t)pre);

    const char* tstr = lv_subject_get_string(&timerType);
    wiseman_set_timer_mode(tstr && strcmp(tstr, "ON") == 0);
    timekeeper_refresh_gui();  // Refresh main page display after timer mode change

    // Persist orientation (FLIP) mapping OFF->DEFAULT, ON->ROTATED, AUTO->AUTO
    const char* orient = lv_subject_get_string(&orientation);
    wiseman_orientation_t o = WISEMAN_ORIENTATION_DEFAULT;
    if (orient) {
        if (strcmp(orient, "ON") == 0) o = WISEMAN_ORIENTATION_ROTATED;
        else if (strcmp(orient, "AUTO") == 0) o = WISEMAN_ORIENTATION_AUTO;
        else o = WISEMAN_ORIENTATION_DEFAULT; // OFF
    }
    wiseman_set_screen_orientation(o);
    wiseman_set_heaters_enabled(ch1, ch2);

    wiseman_request_flush();
}
