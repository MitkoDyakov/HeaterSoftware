/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "driver/gpio.h"
#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"

#include "esp_log.h"

const static char *TAG = "DISPLAY";

#define PIN_NUM_MOSI 37 
#define PIN_NUM_CLK  36 
#define PIN_NUM_CS   35
#define PIN_NUM_DC   38
#define PIN_NUM_RST  39 

#define BACKLIGHT_CONTROL_PIN   (10)

#define IMAGE_WIDTH  240
#define IMAGE_HEIGHT 135
#define JPG_BPP      2        // RGB565 = 2 bytes

esp_lcd_panel_handle_t panel_handle = NULL; // Global panel handle
static void *lvBuffer1;
static void *lvBuffer2;
#define draw_buffer_sz (240 * 10)          // Buffer for 10 rows

static void lv_tick_cb(void* arg) {
    (void)arg;
    lv_tick_inc(1); // called every 1 ms by esp_timer
}

void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) 
{    
    lv_draw_sw_rgb565_swap(px_map, (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1));
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
}

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_disp_flush_ready(disp);
    return false;
}

void lvgl_init_display(void)
{
    lvBuffer1 = spi_bus_dma_memory_alloc(SPI2_HOST, draw_buffer_sz, 0);
    lvBuffer2 = spi_bus_dma_memory_alloc(SPI2_HOST, draw_buffer_sz, 0);

    // Initialize the SPI bus
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_CLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 10 * 2 + 8
    };
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    // Initialize panel IO
    esp_lcd_panel_io_handle_t io_handle = NULL;

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = 80 * 1000 * 1000, // Use 40 MHz for stability
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8
    };
    esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle);

    // Initialize the display panel
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space = LCD_RGB_ENDIAN_RGB, // Use BGR color space
        .bits_per_pixel = 16
    };
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);

    // Reset and initialize the panel
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_set_gap(panel_handle, 40, 53);
    esp_lcd_panel_invert_color(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, false);
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    // Initialize backlight
    gpio_reset_pin(BACKLIGHT_CONTROL_PIN);
    gpio_set_direction(BACKLIGHT_CONTROL_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BACKLIGHT_CONTROL_PIN, 1); // Turn on backlight
    
    // Create LVGL display
    lv_display_t *lvDisplay = lv_display_create(135, 240);
    lv_display_set_rotation(lvDisplay, LV_DISPLAY_ROTATION_90);
    lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565); // Use RGB565
    lv_display_set_flush_cb(lvDisplay, lvgl_flush_cb);
    lv_display_set_buffers(lvDisplay, (void *)lvBuffer1, lvBuffer2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);

    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, lvDisplay);
}

void app_main(void)
{
    // Initialize LVGL
    lv_init();

    // start a 1ms periodic esp_timer to advance LVGL tick
    esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_cb,
        .arg = NULL,
        .name = "lv_tick"
    };
    esp_timer_handle_t lv_tick_timer;
    esp_timer_create(&periodic_timer_args, &lv_tick_timer);
    esp_timer_start_periodic(lv_tick_timer, 1000); // 1000 us = 1 ms

    // Initialize the display
    lvgl_init_display();

    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello, LVGL!");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *circle;
    lv_style_t circle_style;
    const uint32_t circle_radius = 15;

    lv_style_init(&circle_style);
    lv_style_set_radius(&circle_style, circle_radius);
    lv_style_set_bg_opa(&circle_style, LV_OPA_100);
    lv_color_t color;
    color.red = 0xFF;
    lv_style_set_bg_color(&circle_style, color);
    lv_style_set_border_color(&circle_style, color);

    // Create an object with the new style
    circle = lv_obj_create(lv_scr_act());
    lv_obj_set_size(circle, circle_radius * 2, circle_radius * 2);
    lv_obj_add_style(circle, &circle_style, 0);
    lv_obj_align(circle, LV_ALIGN_CENTER, 0, 0);

    //xTaskCreatePinnedToCore(spin_task, "pinned_task0_core0", 4096, (void*)task_id0, TASK_PRIO_3, NULL, CORE0);
    static uint64_t last_change = 0;
    uint8_t flip = 0;
    while (1) 
    {
        uint64_t now = esp_timer_get_time();
        if ((now - last_change) > 1000000) { // 1 second interval
            if(flip)
            {
                flip = 0;
                lv_obj_set_style_bg_color(circle, lv_color_make(255, 0, 0), 0);
                lv_obj_set_style_border_color(circle, lv_color_make(255, 0, 0), 0);
            }else{
                flip = 1;
                lv_obj_set_style_bg_color(circle, lv_color_make(0, 0, 255), 0);
                lv_obj_set_style_border_color(circle, lv_color_make(0, 0, 255), 0);
            }

            lv_obj_invalidate(circle);
            last_change = now;
        }

        lv_timer_handler(); // Handle LVGL tasks
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    esp_restart();
}
