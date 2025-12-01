#include "main_page.h"
#include "HeaterGUI_gen.h"
#include "fireman/fireman.h"
#include "mailman/mailman.h"
#include "wiseman/wiseman.h"
#include "timekeeper.h"
#include "esp_log.h"
#include <string.h>
#include "pinout.h"

// External global variables from director.c
extern uint8_t opStat; // 0=stopped, 1=running
extern QueueHandle_t g_i2c_queue;
extern bool start_heater;
// Timer edit mode state
static bool s_timer_edit_mode = false;

// Preheat timer handle
#include "freertos/timers.h"
static TimerHandle_t s_preheat_timer = NULL;
static int s_preheat_target = 0;

static void preheat_timer_cb(TimerHandle_t xTimer) {
    // Only transition if opStat is still running (STOP not pressed during preheat)
    extern uint8_t opStat;
    if (opStat) {
        start_heater = true;
    }
    if (s_preheat_timer) {
        xTimerDelete(s_preheat_timer, 0);
        s_preheat_timer = NULL;
    }
}

// Cached PD capabilities from director init
static ap33772s_caps_t s_pd_caps = {0};

// Forward declaration for helper function
static bool request_pd_voltage(uint8_t voltage);

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

/* Helper: request PD voltage change through mailman */
static bool request_pd_voltage(uint8_t voltage) {
    if (!g_i2c_queue || (voltage != 5 && voltage != 9 && voltage != 15 && voltage != 20)) {
        return false;
    }
    i2c_msg_t msg = {0};
    msg.type = I2C_MSG_PD_SET_PDO;
    msg.data.pd_set.set_voltage = voltage;
    QueueHandle_t resp = xQueueCreate(1, sizeof(bool));
    if (!resp) return false;
    msg.response_queue = resp;
    bool ok = false;
    if (xQueueSend(g_i2c_queue, &msg, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (xQueueReceive(resp, &ok, pdMS_TO_TICKS(200)) == pdTRUE) {
            // Success - response received
        } else {
            ok = false; // timeout
        }
    }
    vQueueDelete(resp);
    return ok;
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
            opStat = !opStat;
            if (opStat) {
                // Validate timer mode before starting anything
                if(timekeeper_is_timekeeper_mode_set() && !timekeeper_is_timer_set()) {
                    opStat = 0; // Reset opStat since we're not actually starting
                    return;
                }
                
                int preheat = lv_subject_get_int(&preHeat);
                if (preheat == 0) {                   
                    start_heater = true;
                } else {
                   // Preheat phase: set to preheat temp, start one-shot timer                    
                    if (s_preheat_timer) {
                        xTimerDelete(s_preheat_timer, 0);
                        s_preheat_timer = NULL;
                    }
                    s_preheat_timer = xTimerCreate("preheat", preheat * 60 * 1000 / portTICK_PERIOD_MS, pdFALSE, NULL, preheat_timer_cb);
                    if (s_preheat_timer) {
                        xTimerStart(s_preheat_timer, 0);
                    }
                    fireman_set_setpoints(55, 55);
                    fireman_set_heater1_enabled(true);
                    fireman_set_heater2_enabled(true);
                    timekeeper_preheat();
                }
            } else {
                // STOP pressed: always cancel preheat if active, stop everything
                if (s_preheat_timer) {
                    xTimerDelete(s_preheat_timer, 0);
                    s_preheat_timer = NULL;
                }
                start_heater = false;
                timekeeper_stop();
                fireman_set_heater1_enabled(false);
                fireman_set_heater2_enabled(false);
                request_pd_voltage(5);
                lv_subject_set_int(&activePDO, 5);
            }
        } else if (msg.btn_id == BUTTON_RIGHT_TOP) {
            // INCREMENT temperature (short press)
            int t = lv_subject_get_int(&targetTemp) + 1;
            if (t < 61) {
                lv_subject_set_int(&targetTemp, t);
                // Live update while running
                if (opStat) {
                    int clamped = t; if (clamped < 0) clamped = 0; else if (clamped > 60) clamped = 60;
                    fireman_set_setpoints(clamped, clamped);
                }
            }
        } else if (msg.btn_id == BUTTON_RIGHT_CENTER) {
            // DECREMENT temperature (short press)
            int t = lv_subject_get_int(&targetTemp) - 1;
            if (t > -1) {
                lv_subject_set_int(&targetTemp, t);
                // Live update while running
                if (opStat) {
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
                        if (opStat) {
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
                        if (opStat) {
                            int clamped = t; if (clamped < 0) clamped = 0; else if (clamped > 60) clamped = 60;
                            fireman_set_setpoints(clamped, clamped);
                        }
                    }
                }
            } break;            
            case BUTTON_RIGHT_BOTTOM: { // "RIGHT_BOTTOM" (long hold to enter timer edit)
                static uint8_t count = 0;
                if(count == 3){ // after 3 repeats (~3s)
                    if (timer_mode_enabled && !s_timer_edit_mode) {
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
