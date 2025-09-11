// Basic integration test for button input and I2C task.
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "switchboard/user_input.h"
#include "mailman/i2c_task.h"
#include "composer/buzzer.h"
#include "director/director.h"
#include "wiseman/wiseman.h"

static const char *TAG = "APP_MAIN";

//patch -p1 < ../../lvgl_translation_fix_forward.patch
//Copilot Chat: Open in Editor Tab 

// Queues
static QueueHandle_t g_button_queue;
static QueueHandle_t g_i2c_queue;

// ---------------- Button event consumer ----------------
// (Button events now consumed by director GUI task.)

// Helper to send an I2C request and wait for a response of a given type.
// Response queue is created per request to keep interface simple for test code.
static esp_err_t i2c_send_and_wait(i2c_msg_t *msg, void *out_buf, size_t out_size, TickType_t timeout_ticks) {
	QueueHandle_t resp_q = xQueueCreate(1, out_size);
	if (!resp_q) return ESP_ERR_NO_MEM;
	msg->response_queue = resp_q;
	if (xQueueSend(g_i2c_queue, msg, pdMS_TO_TICKS(50)) != pdTRUE) {
		vQueueDelete(resp_q);
		return ESP_ERR_TIMEOUT;
	}
	if (xQueueReceive(resp_q, out_buf, timeout_ticks) != pdTRUE) {
		vQueueDelete(resp_q);
		return ESP_ERR_TIMEOUT;
	}
	vQueueDelete(resp_q);
	return ESP_OK;
}

// ---------------- I2C test sequence ----------------
static void i2c_test_task(void *arg) {
	vTaskDelay(pdMS_TO_TICKS(500)); // allow peripheral setup

	// 1) Request PD fixed voltage to 9V just for test
	{
		bool pd_result = false;
		i2c_msg_t msg = {0};
		msg.type = I2C_MSG_PD_SET_PDO;
		msg.data.pd_set.set_voltage = 9; // valid values: 5, 9, 15, 20
		esp_err_t err = i2c_send_and_wait(&msg, &pd_result, sizeof(pd_result), pdMS_TO_TICKS(1000));
		ESP_LOGI(TAG, "PD SET PDO -> err=%s result=%d", esp_err_to_name(err), (int)pd_result);
	}

	// 2) Read both ADC channels
	{
		adc_result_t adc = {0};
		i2c_msg_t msg = {0};
		msg.type = I2C_MSG_ADC_READ_BOTH;
		esp_err_t err = i2c_send_and_wait(&msg, &adc, sizeof(adc), pdMS_TO_TICKS(1000));
		ESP_LOGI(TAG, "ADC BOTH -> err=%s ch1=%.3f ch2=%.3f", esp_err_to_name(err), adc.chan1, adc.chan2);
	}

	// 3) Read ambient temperature sensor
	{
		ambient_temp_result_t amb = {0};
		i2c_msg_t msg = {0};
		msg.type = I2C_MSG_READ_AMBIENT_TEMP;
		esp_err_t err = i2c_send_and_wait(&msg, &amb, sizeof(amb), pdMS_TO_TICKS(1000));
		ESP_LOGI(TAG, "Ambient Temp -> err=%s T=%.2f C", esp_err_to_name(err), amb.ambientTemp);
	}

	// 4) Periodic ADC poll as demonstration
	while (1) {
		adc_result_t adc = {0};
		i2c_msg_t msg = {0};
		msg.type = I2C_MSG_ADC_READ_BOTH;
		if (i2c_send_and_wait(&msg, &adc, sizeof(adc), pdMS_TO_TICKS(500)) == ESP_OK) {
			ESP_LOGI(TAG, "ADC periodic ch1=%.3f ch2=%.3f", adc.chan1, adc.chan2);
		}
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void app_main(void) {
	// Initialize persistent settings (loads from NVS or applies defaults)
	wiseman_init();

	// Create queues
	g_button_queue = xQueueCreate(32, sizeof(event_msg_t));
	g_i2c_queue    = xQueueCreate(10, sizeof(i2c_msg_t));

	if (!g_button_queue || !g_i2c_queue) {
		ESP_LOGE(TAG, "Failed to create queues");
		return;
	}

	// Start subsystems
	inputdetect_setup(g_button_queue);
	i2c_task_start(g_i2c_queue);
	buzzer_init();

	// Start GUI director (consumes button events)
	director_start(g_button_queue);
	// Keep I2C test for now (optional)
	// xTaskCreate(i2c_test_task, "i2c_test", 4096, NULL, 5, NULL);
}