#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

void timekeeper_init(void);
void timekeeper_start_stopwatch(void);
void timekeeper_stop_stopwatch(void);
bool timekeeper_start_timer(uint8_t hours, uint8_t minutes, void (*callback)(void));
void timekeeper_stop_timer(void);
void timekeeper_timer_start_edit(void);
void timekeeper_timer_stop_edit(void);
void timekeeper_decrement_minute(void);
void timekeeper_increment_minute(void);
void timekeeper_decrement_hour(void);
void timekeeper_increment_hour(void);

#endif // TIMEKEEPER_H