#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

void timekeeper_init(void (*callback)(void));
void timekeeper_start(void);
void timekeeper_stop(void);
void timekeeper_timer_start_edit(void);
void timekeeper_timer_stop_edit(void);
void timekeeper_decrement_minute(void);
void timekeeper_increment_minute(void);
void timekeeper_decrement_hour(void);
void timekeeper_increment_hour(void);
bool timekeeper_did_timer_finish(void);

#endif // TIMEKEEPER_H