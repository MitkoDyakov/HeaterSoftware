#include <stddef.h>
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
    TIMER = 1,
    MESSAGE = 2
};

// Timers switched to LVGL timers for safe UI updates
static lv_timer_t *timekeeper_main_timer = NULL;   // 1s periodic
static lv_timer_t *timekeeper_edit_timer = NULL;   // 300ms blink during edit
static bool timekeeper_running = false;            // track paused/resumed state

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
static bool timekeeper_finished = true;  // Flag to signal timer completion to main loop

void timekeeper_refresh()
{
    if (!timekeeper_running)
    {
        const wiseman_settings_t* settings = wiseman_get();
        if (settings && settings->timer_mode) {
            timekeeper_mode = TIMER;
        } else {
            timekeeper_mode = STOPWATCH;
        }

        switch (timekeeper_mode)
        {
            case STOPWATCH:
                lv_subject_copy_string(&command, "START");
                lv_subject_copy_string(&opTime, "00:00");
                break;
            case TIMER:
                lv_subject_copy_string(&command, "START");
                char text[10];
                snprintf(text, sizeof(text), "%02u:%02u", target_timer_hh, target_timer_mm);
                lv_subject_copy_string(&opTime, text);
                break;
            default:
                break;
        }
    }
}

static void timekeeper_edit_cb(lv_timer_t *t) {
    if (visible_flag && blinkCounter == 4) {
        blinkCounter = 0;
        visible_flag = false;
        lv_subject_set_int(&opTimeVisible, 0);
    } else {
        visible_flag = true;
        lv_subject_set_int(&opTimeVisible, 1);
        // Update display with current timer values
        char text[10];
        snprintf(text, sizeof(text), "%02u:%02u", target_timer_hh, target_timer_mm);
        lv_subject_copy_string(&opTime, text);
    }   
    blinkCounter++;
}

static void timekeeper_cb(lv_timer_t *t) {
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

        // Round up remaining time to show full minute until it's actually elapsed
        // E.g., with 59 seconds left, show 1 minute (not 0)
        uint8_t remaining_hh = (uint8_t)(remaining_seconds / 3600u);
        uint8_t remaining_mm = (uint8_t)((remaining_seconds % 3600u + 59u) / 60u);  // Round up to next minute
        
        // Cap at 59 minutes display
        if (remaining_mm > 59) {
            remaining_mm = 59;
        }

        if (timer_ss & 1) {
            snprintf(text, sizeof(text), "%02u %02u", remaining_hh, remaining_mm);
        } else {
            snprintf(text, sizeof(text), "%02u:%02u", remaining_hh, remaining_mm);
        }

        if (remaining_seconds == 0u) {
            timekeeper_stop();
            return;
        }
    }
        
    lv_subject_copy_string(&opTime, text);
}

void timekeeper_preheat()
{
    lv_subject_copy_string(&command, "");
    lv_subject_copy_string(&opTime, "PREHEATING...");
    timekeeper_finished = false;
}

void timekeeper_refresh_gui()
{
    // Reset internal counters
    timer_ss = 0;
    timer_mm = 0;
    timer_hh = 0;
    // Ensure command shows START and opTime shows 00:00
    lv_subject_copy_string(&command, "START");
    lv_subject_copy_string(&opTime, "00:00");
}

void timekeeper_init()
{
    timekeeper_main_timer = lv_timer_create(timekeeper_cb, TIMEKEEPER_PERIOD_MS, NULL);
    lv_timer_pause(timekeeper_main_timer);
    timekeeper_edit_timer = lv_timer_create(timekeeper_edit_cb, 250, NULL);
    lv_timer_pause(timekeeper_edit_timer);

    timekeeper_running = false;

    timer_ss = 0;
    timer_mm = 0;
    timer_hh = 0;
    target_timer_mm = 0;
    target_timer_hh = 0;

    timekeeper_finished = true;

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

    timekeeper_finished = false;  // Reset completion flag when starting
    lv_timer_resume(timekeeper_main_timer);
    timekeeper_running = true;
    lv_subject_copy_string(&command, "STOP");
}

bool timekeeper_is_timekeeper_mode_set(void){
    const wiseman_settings_t* settings = wiseman_get();
    return (settings && settings->timer_mode);
}

bool timekeeper_is_timer_set(void)
{
    return (target_timer_hh != 0 || target_timer_mm != 0);
}   

void timekeeper_stop(void)
{
    lv_timer_pause(timekeeper_main_timer);
    timekeeper_running = false;
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

    timekeeper_finished = true;  // Reset completion flag
}

void timekeeper_increment_hour(void)
{
    target_timer_hh++;
    if (target_timer_hh > 99)
    {
        target_timer_hh = 0;
    }
}

void timekeeper_decrement_hour(void)
{
    if(target_timer_hh == 0)
    {
        target_timer_hh=99;
    }else{
        target_timer_hh--;
    }    
}

void timekeeper_increment_minute(void)
{
    target_timer_mm++;
    if (target_timer_mm > 59)
    {
        target_timer_mm = 0;
    }
}   
void timekeeper_decrement_minute(void)
{
    if(target_timer_mm == 0)
    {
        target_timer_mm = 59;
    }else{
        target_timer_mm--;
    } 
}   

void timekeeper_timer_start_edit(void)
{
    lv_timer_resume(timekeeper_edit_timer);
}   

void timekeeper_timer_stop_edit(void)
{
    lv_subject_set_int(&opTimeVisible, 1);
    lv_timer_pause(timekeeper_edit_timer);
    // Update display with final timer values before exiting edit mode
    char text[10];
    snprintf(text, sizeof(text), "%02u:%02u", target_timer_hh, target_timer_mm);
    lv_subject_copy_string(&opTime, text);
}

bool timekeeper_is_done(void)
{
    return timekeeper_finished;
}   