#ifndef I2C_TASK_H
#define I2C_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdint.h>

// Result structure for returning both ADC channels
typedef struct {
    double chan1;
    double chan2;
} adc_result_t;

typedef struct {
    float ambientTemp;
} ambient_temp_result_t;

typedef enum {
    I2C_MSG_PD_SET_PDO,     // Set PDO (voltage/current) on PD controller
    I2C_MSG_ADC_READ_SINGLE_CH,  // Read ADC channel (chan1 or chan2)
    I2C_MSG_ADC_READ_BOTH,  // Read both ADC channels
    I2C_MSG_READ_AMBIENT_TEMP,  // Read ambient temperature
} i2c_msg_type_t;

typedef struct {
    i2c_msg_type_t type;
    union {
        struct { // For I2C_MSG_PD_SET_PDO
            uint8_t set_voltage; // 0=5V, 1=9V, 2=15V, 3=20V
        } pd_set;
        struct { // For I2C_MSG_ADC_READ_CHAN
            adc_result_t channel; // 1 or 2
        } adc_read;
    } data;
    QueueHandle_t response_queue; // For sending result back
} i2c_msg_t;

void i2c_task_start(QueueHandle_t queue);

#endif // I2C_TASK_H
