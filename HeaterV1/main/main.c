// Basic integration test for button input and I2C task.
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "switchboard/switchboard.h"
#include "mailman/mailman.h"
#include "composer/composer.h"
#include "director/director.h"
#include "wiseman/wiseman.h"
#include "fireman/fireman.h"
#include "freertos/semphr.h"

static const char *TAG = "APP_MAIN";

//patch -p1 < ../../lvgl_translation_fix_forward.patch
//Copilot Chat: Open in Editor Tab 

// Queues
static QueueHandle_t g_button_queue;
static QueueHandle_t g_i2c_queue;
static QueueHandle_t g_temperature_queue;

// ---------------- Button event consumer ----------------
// (Button events now consumed by director GUI task.)

void app_main(void) {
	// Create queues
	ap33772s_caps_t pd_caps = {0};

	g_button_queue      = xQueueCreate(32, sizeof(event_msg_t));
	g_i2c_queue         = xQueueCreate(10, sizeof(i2c_msg_t));
	g_temperature_queue = xQueueCreate(1,  sizeof(fireman_sample_t)); // single-slot latest sample

	if (!g_button_queue || !g_i2c_queue || !g_temperature_queue) {
		ESP_LOGE(TAG, "Failed to create queues");
		return;
	}

	// Initialize persistent settings (loads from NVS or applies defaults)
	wiseman_init();
	// User button input handling
	switchboard_init(g_button_queue);
	// i2c communication and device management
	mailman_init(g_i2c_queue, &pd_caps);
	// temperature regulation and heater control
	fireman_init(g_i2c_queue, g_temperature_queue);
	// GUI task
	director_init(g_button_queue, g_temperature_queue, g_i2c_queue, &pd_caps);
	// beep in the end :)
	composer_init();
}