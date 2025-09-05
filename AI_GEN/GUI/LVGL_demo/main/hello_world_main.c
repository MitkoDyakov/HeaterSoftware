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

#include "ui.h"
#include "demo_gen.h"
#include "home_gen.h"
#include "HeaterGUI_gen.h"

extern lv_obj_t *column_1;

static void load_page(uint8_t page_id) {
    // Clear old content
    lv_obj_clean(column_1);

    switch(page_id) {
    case 0: {

        lv_obj_t * row_4 = row_create(column_1);
        lv_obj_set_width(row_4, 141);
        lv_obj_set_height(row_4, 83);

        lv_obj_t * target_tmp_0 = target_tmp_create(row_4, &targetTemp);
        lv_obj_set_style_pad_all(target_tmp_0, 0, 0);

        lv_obj_t * row_5 = row_create(column_1);
        lv_obj_set_width(row_5, 141);
        lv_obj_set_height(row_5, 39);
        lv_obj_set_style_margin_top(row_5, 4, 0);

        lv_obj_t * control_0 = control_create(row_5, &command, &opTime);
        lv_obj_set_style_pad_all(control_0, 0, 0);

    } break;

    case 1: {
        lv_obj_t * info_0 = info_create(column_1, "Page 2");
    } break;

    case 2: {
        lv_obj_t * info_0 = info_create(column_1, "Page 3");
    } break;

    case 3: {
        lv_obj_t * info_0 = info_create(column_1, "Page 4");
    } break;
    }
}

// patch -p1 < ../../lvgl_translation_fix_forward.patch
const static char *TAG = "DISPLAY";

__attribute__((weak)) void lv_obj_set_name(lv_obj_t *obj, const char *name)
{
    LV_UNUSED(obj);
    LV_UNUSED(name);
    // No-op: your UI generator can call this safely; nothing happens.
}

// ---------- Tuning ----------
#define DEBOUNCE_MS             30
#define INITIAL_REPEAT_DELAY_MS 400
#define REPEAT_MS               400
#define TIMER_PERIOD_MS         20   // button scan cadence
#define NUM_BUTTONS             6

// We only keep the 1ms LVGL tick esp_timer. The "clock" uses an LVGL timer now.
static esp_timer_handle_t lv_tick_timer = NULL;

// op time counters
static uint8_t timer_ss = 0;
static uint8_t timer_mm = 0;
static uint8_t timer_hh = 0;

// ---------- Events ----------
typedef enum {
    BUTTON_EVENT_SHORT,
    BUTTON_EVENT_REPEAT
} button_event_t;

typedef struct {
    int            btn_id;   // GPIO number (or your own ID)
    button_event_t event;
} event_msg_t;

// ---------- Button state ----------
typedef struct {
    int        gpio;
    const char *name;

    // debounced stable level (active-low: 0=pressed, 1=released)
    int        stable_level;
    bool       pressed;

    // debounce edge gating (written in ISR, read in timer task)
    volatile bool       pending;              // ISR saw an edge
    volatile TickType_t debounce_deadline;    // when to re-sample

    // repeat & short suppression
    TickType_t press_start;
    TickType_t last_repeat;
    bool       any_repeat_since_press;
} button_t;

// ---------- YOUR BUTTONS ----------
static button_t buttons[NUM_BUTTONS] = {
    { .gpio = 40, .name = "RIGHT_BOTTOM"},
    { .gpio = 41, .name = "RIGHT_TOP"},
    { .gpio = 42, .name = "RIGHT_CENTER"},
    { .gpio = 45, .name = "LEFT_BOTTOM"},
    { .gpio = 47, .name = "LEFT_CENTER"},
    { .gpio = 48, .name = "LEFT_TOP"}
};

static QueueHandle_t event_queue;
static TimerHandle_t scan_timer;

// ---- tick helpers (wrap-safe) ----
static inline bool tick_reached(TickType_t now, TickType_t deadline) {
    return (TickType_t)(now - deadline) < (TickType_t)0x80000000;
}

// ---------- Emit event ----------
static inline void emit_event(int gpio, button_event_t ev) {
    event_msg_t msg = { .btn_id = gpio, .event = ev };
    (void)xQueueSend(event_queue, &msg, 0);
}

// ---------- ISR: mark edge + set debounce deadline ----------
static void IRAM_ATTR button_isr_handler(void *arg) {
    button_t *btn = (button_t *)arg;
    TickType_t now = xTaskGetTickCountFromISR();
    btn->pending = true;
    btn->debounce_deadline = now + pdMS_TO_TICKS(DEBOUNCE_MS);
}

// ---------- Timer callback: debounce + repeat + short ----------
static void scan_timer_callback(TimerHandle_t t) {
    (void)t;
    TickType_t now = xTaskGetTickCount();

    for (int i = 0; i < NUM_BUTTONS; i++) {
        button_t *btn = &buttons[i];

        // 1) Debounced edge processing (only after deadline)
        if (btn->pending && tick_reached(now, btn->debounce_deadline)) {
            btn->pending = false;
            int level = gpio_get_level(btn->gpio); // 0=pressed (active-low), 1=released

            if (level != btn->stable_level) {
                btn->stable_level = level;

                if (level == 0 && !btn->pressed) {
                    // PRESSED
                    btn->pressed = true;
                    btn->press_start = now;
                    btn->last_repeat = now;         // base for initial repeat delay
                    btn->any_repeat_since_press = false;
                } else if (level == 1 && btn->pressed) {
                    // RELEASED
                    btn->pressed = false;

                    // If no repeat during this hold -> it's a SHORT
                    if (!btn->any_repeat_since_press) {
                        emit_event(btn->gpio, BUTTON_EVENT_SHORT);
                    }
                    // reset flag for next press
                    btn->any_repeat_since_press = false;
                }
            }
        }

        // 2) Repeat-on-hold (keyboard style): runs every tick
        if (btn->pressed) {
            TickType_t elapsed_ms = (now - btn->last_repeat) * portTICK_PERIOD_MS;
            TickType_t due_ms     = (btn->last_repeat == btn->press_start)
                                    ? INITIAL_REPEAT_DELAY_MS
                                    : REPEAT_MS;

            if (elapsed_ms >= due_ms) {
                emit_event(btn->gpio, BUTTON_EVENT_REPEAT);
                btn->last_repeat = now;
                btn->any_repeat_since_press = true; // suppress SHORT at release
            }
        }
    }
}

// ===================== Display & LVGL =====================

#define PIN_NUM_MOSI 37
#define PIN_NUM_CLK  36
#define PIN_NUM_CS   35
#define PIN_NUM_DC   38
#define PIN_NUM_RST  39

#define BACKLIGHT_CONTROL_PIN   (10)

#define IMAGE_WIDTH  240
#define IMAGE_HEIGHT 135
#define JPG_BPP      2        // RGB565 = 2 bytes

esp_lcd_panel_handle_t panel_handle = NULL; // Global panel handle
static void *lvBuffer1;
static void *lvBuffer2;
#define draw_buffer_sz (IMAGE_WIDTH * IMAGE_HEIGHT * JPG_BPP)          // Buffer for 10 rows

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
        .max_transfer_sz = draw_buffer_sz
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

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
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle));

    // Initialize the display panel
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    // Reset and initialize the panel
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 40, 53));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // Initialize backlight
    gpio_reset_pin(BACKLIGHT_CONTROL_PIN);
    gpio_set_direction(BACKLIGHT_CONTROL_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BACKLIGHT_CONTROL_PIN, 1); // Turn on backlight

    // Create LVGL display
    lv_display_t *lvDisplay = lv_display_create(135, 240);
    lv_display_set_rotation(lvDisplay, LV_DISPLAY_ROTATION_90);
    lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565); // Use RGB565
    lv_display_set_flush_cb(lvDisplay, lvgl_flush_cb);
    lv_display_set_buffers(lvDisplay, (void *)lvBuffer1, (void *)lvBuffer2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);

    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, lvDisplay));
}

// ===================== UI Clock via LVGL timer =====================
static lv_timer_t *ui_clock_timer = NULL;

static void ui_clock_cb(lv_timer_t *t) {
    (void)t;
    // update counters
    timer_ss++;
    if (timer_ss == 60) { timer_ss = 0; timer_mm++; }
    if (timer_mm == 60) { timer_mm = 0; timer_hh++; }
    if (timer_hh == 99) { timer_hh = 0; }

    // update label (safe: LVGL task)
    char text[10];
    if (timer_ss & 1)  snprintf(text, sizeof(text), "%02u %02u", timer_hh, timer_mm);
    else               snprintf(text, sizeof(text), "%02u:%02u", timer_hh, timer_mm);
    lv_subject_copy_string(&opTime, text);
}

void app_main(void)
{
    // Initialize LVGL
    lv_init();

    // 1ms periodic esp_timer to advance LVGL tick (OK to call from timer)
    esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_cb,
        .arg = NULL,
        .name = "lv_tick"
    };
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &lv_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lv_tick_timer, 1000)); // 1000 us = 1 ms

    lvgl_init_display();

    ui_init(NULL);
    lv_obj_t * home_screen = home_create();
    lv_scr_load(home_screen);

    // Create the LVGL clock timer (start paused; we control via button)
    ui_clock_timer = lv_timer_create(ui_clock_cb, 1000, NULL);
    lv_timer_pause(ui_clock_timer);

    // GPIO setup
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    for (int i = 0; i < NUM_BUTTONS; i++) {
        button_t *btn = &buttons[i];

        gpio_config_t io = {
            .intr_type = GPIO_INTR_ANYEDGE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << btn->gpio),
            .pull_up_en = GPIO_PULLUP_ENABLE,    // active-low buttons
            .pull_down_en = GPIO_PULLDOWN_DISABLE
        };
        ESP_ERROR_CHECK(gpio_config(&io));

        // Initialize stable state from hardware
        btn->stable_level = gpio_get_level(btn->gpio);     // 0 pressed, 1 released
        btn->pressed = (btn->stable_level == 0);
        btn->pending = false;
        btn->debounce_deadline = 0;

        // Attach ISR
        ESP_ERROR_CHECK(gpio_isr_handler_add(btn->gpio, button_isr_handler, btn));
    }

    // Event queue
    event_queue = xQueueCreate(32, sizeof(event_msg_t));

    // Single periodic software timer for scanning
    scan_timer = xTimerCreate("btn_scan",
                              pdMS_TO_TICKS(TIMER_PERIOD_MS),
                              pdTRUE, NULL,
                              scan_timer_callback);
    xTimerStart(scan_timer, 0);

    // State used in button handler
    bool opStat = false;
    bool chan1 = true;
    bool chan2 = true;

    // ========= Main UI task loop (sole owner of LVGL calls) =========
    while (1)
    {
        // Drain button events here (replaces vConsoleTask)
        event_msg_t msg;
        while (xQueueReceive(event_queue, &msg, 0) == pdTRUE) {
            if (msg.event == BUTTON_EVENT_SHORT) {
                switch (msg.btn_id) {
                    case 40: { // "RIGHT_BOTTOM"
                        opStat = !opStat;
                        if (opStat) {
                            lv_subject_copy_string(&command, "STOP");
                            lv_timer_resume(ui_clock_timer);
                        } else {
                            lv_subject_copy_string(&command, "START");
                            timer_ss = timer_mm = timer_hh = 0;
                            lv_subject_copy_string(&opTime, "00:00");
                            lv_timer_pause(ui_clock_timer);
                        }
                    } break;

                    case 41: { // "RIGHT_TOP"
                        int t = lv_subject_get_int(&targetTemp);
                        t++;
                        if (t > 60) t = 60;
                        lv_subject_set_int(&targetTemp, t);
                    } break;

                    case 42: { // "RIGHT_CENTER"
                        int t = lv_subject_get_int(&targetTemp);
                        t--;
                        if (t < 0) t = 0;
                        lv_subject_set_int(&targetTemp, t);
                    } break;

                    case 45: { // "LEFT_BOTTOM"
                        int t = lv_subject_get_int(&pageSelect);
                        t++;
                        if (t > 3) t = 0;
                        lv_subject_set_int(&pageSelect, t);
                        load_page(t);
                    } break;

                    case 47: { // "LEFT_CENTER"
                        chan2 = !chan2;
                        lv_subject_copy_string(&ch2_active, chan2 ? "•" : " ");
                    } break;

                    case 48: { // "LEFT_TOP"
                        chan1 = !chan1;
                        lv_subject_copy_string(&ch1_active, chan1 ? "•" : " ");
                    } break;
                }
            } else { // BUTTON_EVENT_REPEAT
                switch (msg.btn_id) {
                    case 41: { // "RIGHT_TOP"
                        int t = lv_subject_get_int(&targetTemp);
                        t++;
                        if (t > 60) t = 60;
                        lv_subject_set_int(&targetTemp, t);
                    } break;
                    case 42: { // "RIGHT_CENTER"
                        int t = lv_subject_get_int(&targetTemp);
                        t--;
                        if (t < 0) t = 0;
                        lv_subject_set_int(&targetTemp, t);
                    } break;
                    default:
                        break;
                }
            }
        }

        // Run LVGL
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    // Not reached
    // esp_restart();
}