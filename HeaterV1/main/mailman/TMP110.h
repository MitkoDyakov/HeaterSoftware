#ifndef TMP110_H
#define TMP110_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c_master.h"

#define TMP110_ADDRESS 0x4a

#define TMP110_TEMP_REG        (0x00)
#define TMP110_CONF_REG        (0x01)
#define TMP110_TEMP_AD_STEP	  (0.0625)

bool TMP110_setup(i2c_master_dev_handle_t device);
bool TMP110_getTemp(float *temp);

#endif // TMP110_H