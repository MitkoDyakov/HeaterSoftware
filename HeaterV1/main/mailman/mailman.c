#include "driver/i2c_master.h"
#include "mailman.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
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
static i2c_master_bus_handle_t g_bus_handle = NULL;

// Removed PD_GET_PDOS: discovery/config happens at startup per design

static void handle_adc_read(i2c_msg_t *msg) {
    adc_result_t result;
    getTemperature(&result.chan1, &result.chan2);
    xQueueSend(msg->response_queue, &result, 0);
}

static void handle_pd_set_pdo(i2c_msg_t *msg) {
    // Set PDO using DMA, send result to response queue
    bool ret = AP33772S_setFixPDO(msg->data.pd_set.set_voltage);
    xQueueSend(msg->response_queue, &ret, 0);
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
    float ambient_temp;
    TMP110_getTemp(&ambient_temp);
    xQueueSend(msg->response_queue, &ambient_temp, 0);
}

static void handle_pd_get_current(i2c_msg_t *msg) {
    uint16_t current_ma = AP33772S_getCurrent();
    xQueueSend(msg->response_queue, &current_ma, 0);
}

static void handle_pd_get_temp(i2c_msg_t *msg) {
    uint8_t temp_c = APS33772S_getTemperature();
    xQueueSend(msg->response_queue, &temp_c, 0);
}

static void mailman_task(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    i2c_msg_t msg;

    // --- Main loop: handle runtime requests ---
    while (1) {
        if (xQueueReceive(queue, &msg, portMAX_DELAY)) {
            switch (msg.type) {
                case I2C_MSG_PD_SET_PDO:
                    handle_pd_set_pdo(&msg);
                    break;
                case I2C_MSG_PD_GET_CURRENT:
                    handle_pd_get_current(&msg);
                    break;
                case I2C_MSG_PD_GET_TEMP:
                    handle_pd_get_temp(&msg);
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

void mailman_init(QueueHandle_t queue, ap33772s_caps_t *pd_caps) {
    // I2C bus setup (new API)
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .glitch_ignore_cnt = 0,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &g_bus_handle));

    i2c_device_config_t ads7142_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ADS7142_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(g_bus_handle, &ads7142_cfg, &ads7142_handle));

    i2c_device_config_t ap33772s_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AP33772S_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(g_bus_handle, &ap33772s_cfg, &ap33772s_handle));

    i2c_device_config_t TMP110_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TMP110_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(g_bus_handle, &TMP110_cfg, &TMP110_handle));
    
    // Device setup (all synchronous)
    AP33772S_setup(ap33772s_handle);
    ADS7142_setup(ads7142_handle);
    TMP110_setup(TMP110_handle);
    
    // Get PD capabilities (now cached by AP33772S_setup)
    if (pd_caps) {
        ap33772s_get_caps(pd_caps);
    }
    
    xTaskCreate(mailman_task, "mailman_task", 4096, (void *)queue, 10, NULL);
}


