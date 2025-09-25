#ifndef __FIREMAN_H__
#define __FIREMAN_H__

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

// Start background Fireman task (temperature polling + PID). Provide I2C request queue and a
// jumbotron/display queue of length 1. Fireman uses xQueueOverwrite to always publish the latest sample.
bool fireman_setup(QueueHandle_t i2c_queue, QueueHandle_t jumbotron_queue);

// Sample pushed to jumbotron/display queue each control period (temperatures Celsius).
// If a PD failure occurs both values may be NAN.
typedef struct fireman_sample_s {
    double ch1;
    double ch2;
} fireman_sample_t;

// Current active PD voltage (5,9,15,20) as last successfully requested (exposed for UI).
int fireman_get_current_pd_voltage(void);

// Enable/disable heaters independently. If both disabled PID loop idles.
void fireman_set_heater1_enabled(bool en);
void fireman_set_heater2_enabled(bool en);

// Set target temperature(s) in deg C
void fireman_set_setpoint1(int setpoint_c);
void fireman_set_setpoint2(int setpoint_c);
void fireman_set_setpoints(int sp1_c, int sp2_c);

// Request a PD fixed voltage (5,9,15,20). Returns true on success, false on failure.
// Uses the same internal mechanism as Fireman's own PD management.
bool fireman_request_pd_voltage(uint8_t voltage);

#endif