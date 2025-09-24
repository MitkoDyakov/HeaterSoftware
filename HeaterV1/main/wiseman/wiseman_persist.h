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

// Statistics for debugging persistence behavior.
typedef struct {
	uint32_t saves_attempted;
	uint32_t saves_ok;
	uint32_t last_latency_ms;
	uint32_t max_latency_ms;
	uint32_t forced_flushes;
	uint32_t debounced_marks;  // marks ignored because already dirty
	uint32_t task_stack_low_water; // words remaining (minimum ever)
} wiseman_persist_stats_t;

// Returns a snapshot of current persistence stats (zeros if disabled or not started).
void wiseman_persist_get_stats(wiseman_persist_stats_t* out);

#ifdef __cplusplus
}
#endif
