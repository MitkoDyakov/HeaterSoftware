// Button input handling (debounce + repeat events).
#include "user_input.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"

// ---------- Button state ----------
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

void inputdetect_setup(QueueHandle_t external_queue)
{
    event_queue = external_queue;

    // GPIO setup (allow already-installed state)
    esp_err_t err = gpio_install_isr_service(0);

    for (int i = 0; i < NUM_BUTTONS; i++) {
        button_t *btn = &buttons[i];

        gpio_config_t io = {
            .intr_type = GPIO_INTR_ANYEDGE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1ULL << btn->gpio),
            .pull_up_en = GPIO_PULLUP_ENABLE,    // active-low buttons
            .pull_down_en = GPIO_PULLDOWN_DISABLE
        };
        gpio_config(&io);

        // Initialize stable state from hardware
        btn->stable_level = gpio_get_level(btn->gpio);     // 0 pressed, 1 released
        btn->pressed = (btn->stable_level == 0);

        // Attach ISR
        gpio_isr_handler_add(btn->gpio, button_isr_handler, btn);
    }

    // Single periodic timer
    scan_timer = xTimerCreate("btn_scan",
                            pdMS_TO_TICKS(TIMER_PERIOD_MS),
                            pdTRUE, NULL,
                            scan_timer_callback);

    xTimerStart(scan_timer, 0);
}
