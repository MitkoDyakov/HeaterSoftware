// SPDX-License-Identifier: MIT
#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// ---------- Tuning (mirrors implementation) ----------
#define DEBOUNCE_MS             30
#define INITIAL_REPEAT_DELAY_MS 400
#define REPEAT_MS               400
#define TIMER_PERIOD_MS         20   // timer tick (scan cadence)
#define NUM_BUTTONS             6

// ---------- Button state ----------
typedef struct {
    int        gpio;
    const char *name;

    // debounced stable level (active-low: 0=pressed, 1=released)
    int        stable_level;
    bool       pressed;

    // debounce edge gating
    bool       pending;              // ISR saw an edge
    TickType_t debounce_deadline;    // when to re-sample

    // repeat & short suppression
    TickType_t press_start;
    TickType_t last_repeat;
    bool       any_repeat_since_press;
} button_t;

// ---------- Events ----------
typedef enum {
    BUTTON_EVENT_SHORT,
    BUTTON_EVENT_REPEAT
} button_event_t;

typedef struct {
    int            btn_id;   // GPIO number (or your own ID)
    button_event_t event;
} event_msg_t;

// Setup now requires an externally created queue (item size = sizeof(event_msg_t)).
// Returns immediately (no effect) if queue is NULL.
void inputdetect_setup(QueueHandle_t event_queue);

#endif // USER_INPUT_H
