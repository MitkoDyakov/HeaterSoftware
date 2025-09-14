#include "driver/i2c_master.h"
#include "i2c_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include "pinout.h"

#include "ap33772s.h"
#include "ads7142.h"
#include "TMP110.h"
#include "esp_log.h"

#define I2C_MASTER_PORT      0
#define I2C_MASTER_FREQ_HZ   400000

// Device handles for I2C devices (new driver API)
static i2c_master_dev_handle_t ads7142_handle = NULL;
static i2c_master_dev_handle_t ap33772s_handle = NULL;
static i2c_master_dev_handle_t TMP110_handle = NULL;

// Removed PD_GET_PDOS: discovery/config happens at startup per design

static void handle_adc_read(i2c_msg_t *msg) {
    adc_result_t result;
    getTemperature(&result.chan1, &result.chan2);
    xQueueSend(msg->response_queue, &result, 0);
}

static void handle_pd_set_pdo(i2c_msg_t *msg) {
    // Set PDO using DMA, send result to response queue
    bool ret = setFixPDO(msg->data.pd_set.set_voltage);
    xQueueSend(msg->response_queue, &ret, 0);
}

static void handle_pd_get_caps(i2c_msg_t *msg) {
    i2c_pd_caps_resp_t caps = {0};
    ap33772s_caps_t raw;
    ap33772s_get_caps(&raw);
    ESP_LOGD("pd", "handle_pd_get_caps raw five=%d nine=%d fifteen=%d twenty=%d",
             raw.fiveV, raw.nineV, raw.fifteenV, raw.twentyV);
    caps.have5 = raw.fiveV; caps.cur5 = raw.cur5;
    caps.have9 = raw.nineV; caps.cur9 = raw.cur9;
    caps.have15 = raw.fifteenV; caps.cur15 = raw.cur15;
    caps.have20 = raw.twentyV; caps.cur20 = raw.cur20;
    xQueueSend(msg->response_queue, &caps, 0);
}

static void handle_adc_read_single_ch(i2c_msg_t *msg) {
    // Read ADC channel using DMA, send result to response queue
    adc_result_t result;

    if(msg->data.adc_read.channel== 1){
        getTemperature(&result.chan1, NULL);
    }else{
        getTemperature(NULL, &result.chan2);
    }
     xQueueSend(msg->response_queue, &result, 0);
}

static void read_ambient_temp(i2c_msg_t *msg) {
    ambient_temp_result_t temp = {0};
    TMP110_getTemp(&temp.ambientTemp);
    xQueueSend(msg->response_queue, &temp, 0);
}

static void i2c_task(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    i2c_msg_t msg;

    // I2C bus setup (new API)
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .glitch_ignore_cnt = 0,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t ads7142_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ADS7142_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &ads7142_cfg, &ads7142_handle));

    i2c_device_config_t ap33772s_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AP33772S_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &ap33772s_cfg, &ap33772s_handle));

    i2c_device_config_t TMP110_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TMP110_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &TMP110_cfg, &TMP110_handle));

    AP33772S_setup(ap33772s_handle);
    ADS7142_setup(ads7142_handle);
    TMP110_setup(TMP110_handle);

    // --- Main loop: handle runtime requests ---
    while (1) {
        if (xQueueReceive(queue, &msg, portMAX_DELAY)) {
            switch (msg.type) {
                case I2C_MSG_PD_SET_PDO:
                    handle_pd_set_pdo(&msg);
                    break;
                case I2C_MSG_PD_GET_CAPS:
                    handle_pd_get_caps(&msg);
                    break;
                case I2C_MSG_ADC_READ_SINGLE_CH:
                    handle_adc_read_single_ch(&msg);
                    break;
                case I2C_MSG_ADC_READ_BOTH:
                    handle_adc_read(&msg);
                    break;
                case I2C_MSG_READ_AMBIENT_TEMP:
                    read_ambient_temp(&msg);
                    break;
                default:
                    break;
            }
        }
    }
}

void i2c_task_start(QueueHandle_t queue) {
    xTaskCreate(i2c_task, "i2c_task", 4096, (void *)queue, 10, NULL);
}
