// Button input handling (debounce + repeat events).
#include "switchboard.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "pinout.h"

// ---------- Internal configuration ----------
#define DEBOUNCE_MS             30
#define INITIAL_REPEAT_DELAY_MS 400
#define REPEAT_MS               400
#define TIMER_PERIOD_MS         20   // timer tick (scan cadence)
#define NUM_BUTTONS             6

// ---------- Internal button state ----------
typedef struct {
    int        gpio;
    const char *name;

    // debounced stable level (active-low: 0=pressed, 1=released)
    int        stable_level;
    bool       pressed;

    // debounce edge gating
    bool       pending;              // ISR saw an edge
    TickType_t debounce_deadline;    // when to re-sample

    // repeat & short suppression
    TickType_t press_start;
    TickType_t last_repeat;
    bool       any_repeat_since_press;
} button_t;

static QueueHandle_t event_queue;
static TimerHandle_t scan_timer;

// ---------- YOUR BUTTONS ----------
static button_t buttons[NUM_BUTTONS] = {
    { .gpio = BUTTON_RIGHT_BOTTOM, .name = "RIGHT_BOTTOM"},
    { .gpio = BUTTON_RIGHT_TOP,    .name = "RIGHT_TOP"},
    { .gpio = BUTTON_RIGHT_CENTER, .name = "RIGHT_CENTER"},
    { .gpio = BUTTON_LEFT_BOTTOM,  .name = "LEFT_BOTTOM"},
    { .gpio = BUTTON_LEFT_CENTER,  .name = "LEFT_CENTER"},
    { .gpio = BUTTON_LEFT_TOP,     .name = "LEFT_TOP"}
};

// ---- tick helpers (wrap-safe) ----
static inline bool tick_reached(TickType_t now, TickType_t deadline) {
    return (TickType_t)(now - deadline) < (TickType_t)0x80000000;
}

// ---------- Emit event ----------
static inline void emit_event(int gpio, button_event_t ev) {
    if (!event_queue) return; // Safety check
    
    event_msg_t msg = { .btn_id = gpio, .event = ev };
    BaseType_t result = xQueueSend(event_queue, &msg, 0);
    if (result != pdTRUE) {
        // Queue full - this indicates a system design issue
        ESP_LOGW("SWITCHBOARD", "Event queue full, dropped event for GPIO %d", gpio);
    }
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

esp_err_t switchboard_init(QueueHandle_t external_queue) {
    // Validate parameters
    if (!external_queue) {
        return ESP_ERR_INVALID_ARG;
    }
    
    event_queue = external_queue;

    // GPIO setup (allow already-installed state)
    esp_err_t ret = gpio_install_isr_service(0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        // ESP_ERR_INVALID_STATE means ISR service already installed, which is OK
        return ret;
    }

    for (int i = 0; i < NUM_BUTTONS; i++) {
        button_t *btn = &buttons[i];

        gpio_config_t io = {     
            .intr_type = GPIO_INTR_ANYEDGE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << btn->gpio),
            .pull_up_en = GPIO_PULLUP_ENABLE,    // active-low buttons
            .pull_down_en = GPIO_PULLDOWN_DISABLE
        };
        ret = gpio_config(&io);

        if (ret != ESP_OK) {
            ESP_LOGE("SWITCHBOARD", "Failed to configure GPIO %d: %s", btn->gpio, esp_err_to_name(ret));
            return ret;
        }

        // Initialize stable state from hardware
        btn->stable_level = gpio_get_level(btn->gpio);     // 0 pressed, 1 released
        btn->pressed = (btn->stable_level == 0);

        // Attach ISR
        ret = gpio_isr_handler_add(btn->gpio, button_isr_handler, btn);
        if (ret != ESP_OK) {
            ESP_LOGE("SWITCHBOARD", "Failed to add ISR for GPIO %d: %s", btn->gpio, esp_err_to_name(ret));
            return ret;
        }
    }

    // Single periodic timer
    scan_timer = xTimerCreate("btn_scan",
                            pdMS_TO_TICKS(TIMER_PERIOD_MS),
                            pdTRUE, NULL,
                            scan_timer_callback);
    
    if (!scan_timer) {
        return ESP_ERR_NO_MEM;
    }

    xTimerStart(scan_timer, 0);
    
    return ESP_OK;
}
