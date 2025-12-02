#include "main_page.h"
#include "HeaterGUI_gen.h"
#include "fireman/fireman.h"
#include "mailman/mailman.h"
#include "wiseman/wiseman.h"
#include "timekeeper.h"
#include "esp_log.h"
#include <string.h>
#include "pinout.h"
#include "director/director.h"
// External global variables from director.c
extern QueueHandle_t g_i2c_queue;
// Timer edit mode state

extern volatile main_page_state_t s_heater_state;
extern volatile bool start_heater;
extern volatile bool start_preheating;
extern volatile bool stop_heater;

static bool s_timer_edit_mode = false;
// Cached PD capabilities from director init
static ap33772s_caps_t s_pd_caps = {0};

// Forward declaration for helper function

void page_main_create(lv_obj_t *container, const ap33772s_caps_t *initial_pd_caps) {
    if (initial_pd_caps) {
        s_pd_caps = *initial_pd_caps;
    }
    
    // Create main page UI elements (temperature target and control)
    lv_obj_t *row_4 = row_create(container);
    lv_obj_set_width(row_4, 141);
    lv_obj_set_height(row_4, 83);

    lv_obj_t *target_tmp_0 = target_tmp_create(row_4, &targetTemp);
    lv_obj_set_style_pad_all(target_tmp_0, 0, 0);

    lv_obj_t *row_5 = row_create(container);
    lv_obj_set_width(row_5, 141);
    lv_obj_set_height(row_5, 39);
    lv_obj_set_style_margin_top(row_5, 4, 0);

    lv_obj_t *control_0 = control_create(row_5, &command, &opTime);
    lv_obj_set_style_pad_all(control_0, 0, 0);
}

void page_main_cleanup(void) {
    // Reset timer edit mode when leaving page
    s_timer_edit_mode = false;
}

void main_page_handle_event(event_msg_t msg) {
    // Check if timer mode is enabled in wiseman
    const wiseman_settings_t* settings = wiseman_get();
    bool timer_mode_enabled = (settings && settings->timer_mode);
    
    if (msg.event == BUTTON_EVENT_SHORT) {
        if (s_timer_edit_mode) {
            // In edit mode: handle all buttons
            switch (msg.btn_id) {
                case BUTTON_LEFT_TOP:
                    timekeeper_increment_hour();
                    break;
                case BUTTON_LEFT_CENTER:
                    timekeeper_decrement_hour();
                    break;
                case BUTTON_RIGHT_TOP:
                    timekeeper_increment_minute();
                    break;
                case BUTTON_RIGHT_CENTER:
                    timekeeper_decrement_minute();
                    break;
                case BUTTON_RIGHT_BOTTOM:
                    // Exit edit mode
                    timekeeper_timer_stop_edit();
                    s_timer_edit_mode = false;
                    break;
            }
            return; // Don't process other button logic while in edit mode
        }
        
        if (msg.btn_id == BUTTON_RIGHT_BOTTOM) {
            // Not in timer edit mode: normal start/stop logic
            switch(s_heater_state) {
                case STATE_IDLE:
                    // Validate timer mode before starting
                    if (timekeeper_is_timer_valid() == false) {
                        ESP_LOGI("main_page.timer", "Timer mode enabled but no time set; ignoring start request");
                        return;
                    }
                    int preheat = lv_subject_get_int(&preHeat);
                    ESP_LOGI("main_page.preheat", "START pressed: preHeat=%d", preheat);
                    if (preheat > 0) {
                        ESP_LOGI("main_page.preheat", "Starting preheat for %d minutes", preheat);
                        start_preheating = true;
                    } else {
                        ESP_LOGI("main_page.preheat", "No preheat configured; starting heater immediately");
                        start_heater = true;
                    }
                    break;
                case STATE_PREHEAT:
                    ESP_LOGI("main_page.preheat", "STOP pressed during preheat");
                    stop_heater = true;
                    break;
                case STATE_RUNNING:
                    ESP_LOGI("main_page.preheat", "STOP pressed during running");
                    stop_heater = true;
                    break;
            }
        } else if (msg.btn_id == BUTTON_RIGHT_TOP) {
            // INCREMENT temperature (short press)
            int t = lv_subject_get_int(&targetTemp) + 1;
            if (t < 61) {
                lv_subject_set_int(&targetTemp, t);
                // Live update while running (skip if preheat timer active)
                if (s_heater_state == STATE_RUNNING) {
                    int clamped = t; if (clamped < 0) clamped = 0; else if (clamped > 60) clamped = 60;
                    fireman_set_setpoints(clamped, clamped);
                }
            }
        } else if (msg.btn_id == BUTTON_RIGHT_CENTER) {
            // DECREMENT temperature (short press)
            int t = lv_subject_get_int(&targetTemp) - 1;
            if (t > -1) {
                lv_subject_set_int(&targetTemp, t);
                // Live update while running (skip if preheat timer active)
                if (s_heater_state == STATE_RUNNING) {
                    int clamped = t; if (clamped < 0) clamped = 0; else if (clamped > 60) clamped = 60;
                    fireman_set_setpoints(clamped, clamped);
                }
            }
        }
    } else if (msg.event == BUTTON_EVENT_REPEAT) {
        switch (msg.btn_id) {
            case BUTTON_RIGHT_TOP: { // "RIGHT_TOP" (increment hold)
                if (!s_timer_edit_mode) {
                    int t = lv_subject_get_int(&targetTemp) + 1;
                    if (t < 61) {
                        lv_subject_set_int(&targetTemp, t);
                        if (s_heater_state == STATE_RUNNING) {
                            int clamped = t; if (clamped < 0) clamped = 0; else if (clamped > 60) clamped = 60;
                            fireman_set_setpoints(clamped, clamped);
                        }
                    }
                }
            } break;
            case BUTTON_RIGHT_CENTER: { // "RIGHT_CENTER" (decrement hold)
                if (!s_timer_edit_mode) {
                    int t = lv_subject_get_int(&targetTemp) - 1;
                    if (t > -1) {
                        lv_subject_set_int(&targetTemp, t);
                        if (s_heater_state == STATE_RUNNING) {
                            int clamped = t; if (clamped < 0) clamped = 0; else if (clamped > 60) clamped = 60;
                            fireman_set_setpoints(clamped, clamped);
                        }
                    }
                }
            } break;            
            case BUTTON_RIGHT_BOTTOM: { // "RIGHT_BOTTOM" (long hold to enter timer edit)
                static uint8_t count = 0;
                if(count == 3){ // after 3 repeats (~3s)
                    // Allow entering edit mode only when IDLE
                    if (s_heater_state == STATE_IDLE && timer_mode_enabled && !s_timer_edit_mode) {
                        timekeeper_timer_start_edit();
                        s_timer_edit_mode = true;
                        ESP_LOGI("main_page.timer", "Entered timer edit mode");
                    }
                    count = 0; // reset count
                }
                count++;
            } break;
        }
    }
}
