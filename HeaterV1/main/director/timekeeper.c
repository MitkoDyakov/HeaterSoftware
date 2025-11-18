#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include "lvgl.h"
#include "HeaterGUI_gen.h"

#define TIMEKEEPER_PERIOD_MS 1000

enum {
    STOPWATCH = 0,
    TIMER = 1
};

// UI Clock via LVGL timer
static TimerHandle_t timekeeper_timer = NULL;

// op time counters
static uint8_t timer_ss = 0;
static uint8_t timer_mm = 0;
static uint8_t timer_hh = 0;
static uint8_t target_timer_mm = 0;
static uint8_t target_timer_hh = 0;
static uint8_t timekeeper_mode = STOPWATCH;

void (*callback_function)(void) = NULL;

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
            xTimerStop(timekeeper_timer, 0);
        }
    }

    if (timekeeper_mode == TIMER) {
        uint8_t remaining_hh = target_timer_hh - timer_hh;
        uint8_t remaining_mm = target_timer_mm - timer_mm;

        if (timer_ss & 1) {
            snprintf(text, sizeof(text), "%02u %02u", remaining_hh, remaining_mm);
        } else {
            snprintf(text, sizeof(text), "%02u:%02u", remaining_hh, remaining_mm);
        }

        if (remaining_hh == 0 && remaining_mm == 0) {
            xTimerStop(timekeeper_timer, 0);
            if (callback_function != NULL) {
                callback_function();    
            }
        }
    }
        
    lv_subject_copy_string(&opTime, text);
}

void timekeeper_init(void)
{
    timekeeper_timer = xTimerCreate("timekpr", pdMS_TO_TICKS(TIMEKEEPER_PERIOD_MS), pdTRUE, NULL, timekeeper_cb);
    timer_ss = 0;
    timer_mm = 0;
    timer_hh = 0;
    target_timer_mm = 0;
    target_timer_hh = 0;
    timekeeper_mode = STOPWATCH;
}

void timekeeper_start_stopwatch(void)
{
    timekeeper_mode = STOPWATCH;
    xTimerStart(timekeeper_timer, 0);
}

void timekeeper_stop_stopwatch(void)
{
    xTimerStop(timekeeper_timer, 0);
    timer_ss = 0;
    timer_mm = 0;
    timer_hh = 0;
}

bool timekeeper_start_timer(uint8_t hours, uint8_t minutes, void (*callback)(void))
{
    if (hours > 99)
    {
        hours = 99;
    }
    
    if (minutes > 59) 
    {
        minutes = 59;
    }

    if(hours==0 && minutes==0)
    {
        return false;
    }

    if (callback != NULL)
    {
        callback_function = callback;
    }else{
        return false;
    }

    xTimerStop(timekeeper_timer, 0);
    target_timer_mm = minutes;
    target_timer_hh = hours;
    timekeeper_mode = TIMER;
    xTimerStart(timekeeper_timer, 0);

    return true;
}

void timekeeper_stop_timer(void)
{
    timer_ss = 0;
    timer_mm = 0;
    timer_hh = 0;
    xTimerStop(timekeeper_timer, 0);
}

