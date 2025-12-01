#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

void timekeeper_init();
void timekeeper_start(void);
void timekeeper_stop(void);
void timekeeper_refresh_gui(void);
void timekeeper_timer_start_edit(void);
void timekeeper_timer_stop_edit(void);
void timekeeper_decrement_minute(void);
void timekeeper_increment_minute(void);
void timekeeper_decrement_hour(void);
void timekeeper_increment_hour(void);
bool timekeeper_is_done(void);
void timekeeper_refresh();
bool timekeeper_is_done(void); 
void timekeeper_preheat(void);  
bool timekeeper_is_timer_set(void);
bool timekeeper_is_timekeeper_mode_set(void);

#endif // TIMEKEEPER_H