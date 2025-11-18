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

#endif // TIMEKEEPER_H