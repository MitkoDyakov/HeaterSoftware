#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include "lvgl.h"
#include "HeaterGUI_gen.h"
#include "string.h"
#include "timekeeper.h"
#include "wiseman/wiseman.h"

#define TIMEKEEPER_PERIOD_MS 1000

enum {
    STOPWATCH = 0,
    TIMER = 1
};

// UI Clock via LVGL timer
static TimerHandle_t timekeeper_main_timer = NULL;
static TimerHandle_t timekeeper_edit_timer = NULL;

// op time counters
static uint8_t timer_ss = 0;
static uint8_t timer_mm = 0;
static uint8_t timer_hh = 0;
static uint8_t target_timer_mm = 0;
static uint8_t target_timer_hh = 0;
static uint8_t timekeeper_mode = STOPWATCH;

void (*callback_function)(void) = NULL;

bool visible_flag = false;
uint8_t blinkCounter = 0;
static bool timer_finished = false;  // Flag to signal timer completion to main loop


static void timekeeper_edit_cb(TimerHandle_t t) {
    if (visible_flag && blinkCounter == 4) {
        blinkCounter = 0;
        visible_flag = false;
        lv_subject_set_int(&opTimeVisible, 0);
    } else {
        visible_flag = true;
        lv_subject_set_int(&opTimeVisible, 1);
    }   
    blinkCounter++;
}

static void timekeeper_cb(TimerHandle_t t) {
    (void)t;
    // update counters
    char text[10];
    timer_ss++;

    if (timer_ss == 60) {
         timer_ss = 0; 
         timer_mm++;
    }

    if (timer_mm == 60) { 
        timer_mm = 0;
        timer_hh++; 
    }

    if (timer_hh == 99) { 
        timer_hh = 0; 
    }

    if (timekeeper_mode == STOPWATCH) {
        if (timer_ss & 1) {
            snprintf(text, sizeof(text), "%02u %02u", timer_hh, timer_mm);
        } else {
            snprintf(text, sizeof(text), "%02u:%02u", timer_hh, timer_mm);
        }

        if (timer_hh == 99 && timer_mm == 59) {
            timekeeper_stop();
        }
    }

    if (timekeeper_mode == TIMER) {
        // Proper countdown based on total seconds to avoid underflow/overflow
        uint32_t target_seconds  = (uint32_t)target_timer_hh * 3600u + (uint32_t)target_timer_mm * 60u;
        uint32_t elapsed_seconds = (uint32_t)timer_hh * 3600u + (uint32_t)timer_mm * 60u + (uint32_t)timer_ss;
        uint32_t remaining_seconds = 0;
        if (elapsed_seconds < target_seconds) {
            remaining_seconds = target_seconds - elapsed_seconds;
        } else {
            remaining_seconds = 0;
        }

        uint8_t remaining_hh = (uint8_t)(remaining_seconds / 3600u);
        uint8_t remaining_mm = (uint8_t)((remaining_seconds % 3600u) / 60u);

        if (timer_ss & 1) {
            snprintf(text, sizeof(text), "%02u %02u", remaining_hh, remaining_mm);
        } else {
            snprintf(text, sizeof(text), "%02u:%02u", remaining_hh, remaining_mm);
        }

        if (remaining_seconds == 0u) {
            timekeeper_stop();
            timer_finished = true;  // Signal completion to main loop
            return;
        }
    }
        
    lv_subject_copy_string(&opTime, text);
}

void timekeeper_init()
{
    timekeeper_main_timer = xTimerCreate("timekpr", pdMS_TO_TICKS(TIMEKEEPER_PERIOD_MS), pdTRUE, NULL, timekeeper_cb);
    timekeeper_edit_timer = xTimerCreate("timekpr_edit", pdMS_TO_TICKS(300), pdTRUE, NULL, timekeeper_edit_cb);

    timer_ss = 0;
    timer_mm = 0;
    timer_hh = 0;
    target_timer_mm = 0;
    target_timer_hh = 0;

    // Check wiseman settings: if timer_mode is enabled, initialize as timer; otherwise stopwatch
    const wiseman_settings_t* settings = wiseman_get();
    if (settings && settings->timer_mode) {
        timekeeper_mode = TIMER;
    } else {
        timekeeper_mode = STOPWATCH;
    }
}

void timekeeper_start(void)
{
    // Check wiseman settings to determine mode
    const wiseman_settings_t* settings = wiseman_get();
    timekeeper_mode = (settings && settings->timer_mode) ? TIMER : STOPWATCH;
    
    if (timekeeper_mode == TIMER) 
    {
        if( target_timer_hh == 0 && target_timer_mm == 0)
        {
            return;
        }   
    }

    timer_finished = false;  // Reset completion flag when starting
    xTimerStart(timekeeper_main_timer, 0);
    lv_subject_copy_string(&command, "STOP");
}

void timekeeper_stop(void)
{
    xTimerStop(timekeeper_main_timer, 0);
    timer_ss = 0;
    timer_mm = 0;
    timer_hh = 0;
    lv_subject_copy_string(&command, "START");
    
    char text[10];
    if (timekeeper_mode == TIMER) {
        snprintf(text, sizeof(text), "%02u:%02u", target_timer_hh, target_timer_mm);
    } else {
        snprintf(text, sizeof(text), "00:00");
    }
    lv_subject_copy_string(&opTime, text);
}

void timekeeper_increment_hour(void)
{
    char text[10];
    target_timer_hh++;
    if (target_timer_hh > 99)
    {
        target_timer_hh = 0;
    }

    snprintf(text, sizeof(text), "%02u:%02u", target_timer_hh, target_timer_mm);
    lv_subject_copy_string(&opTime, text);
}

void timekeeper_decrement_hour(void)
{
    char text[10];

    if(target_timer_hh == 0)
    {
        target_timer_hh=99;
    }else{
        target_timer_hh--;
    }    
    snprintf(text, sizeof(text), "%02u:%02u", target_timer_hh, target_timer_mm);
    lv_subject_copy_string(&opTime, text);
}

void timekeeper_increment_minute(void)
{
    char text[10];
    target_timer_mm++;
    if (target_timer_mm > 59)
    {
        target_timer_mm = 0;
    }

    snprintf(text, sizeof(text), "%02u:%02u", target_timer_hh, target_timer_mm);
    lv_subject_copy_string(&opTime, text);
}   
void timekeeper_decrement_minute(void)
{
    char text[10];
    if(target_timer_mm == 0)
    {
        target_timer_mm = 59;
    }else{
        target_timer_mm--;
    } 
    snprintf(text, sizeof(text), "%02u:%02u", target_timer_hh, target_timer_mm);
    lv_subject_copy_string(&opTime, text);
}   

void timekeeper_timer_start_edit(void)
{
    xTimerStart(timekeeper_edit_timer, 0);
}   

void timekeeper_timer_stop_edit(void)
{
    lv_subject_set_int(&opTimeVisible, 1);
    xTimerStop(timekeeper_edit_timer, 0);   
}

bool timekeeper_did_timer_finish(void)
{
    return timer_finished;
}   