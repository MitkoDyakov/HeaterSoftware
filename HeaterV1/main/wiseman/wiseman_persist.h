#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// Start background persistence task (idempotent). Returns true on success.
bool wiseman_persist_start(void);

// Mark settings dirty. A debounced save will occur after inactivity window.
void wiseman_mark_dirty(void);

// Force an immediate save from the persistence task context on next cycle.
void wiseman_request_flush(void);

#ifdef __cplusplus
}
#endif
