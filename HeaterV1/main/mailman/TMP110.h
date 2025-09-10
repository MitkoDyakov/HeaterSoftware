#ifndef TMP110_H
#define TMP110_H

#define TMP110_ADDRESS 0x4a

#define TMP75_TEMP_REG        (0x00)
#define TMP75_CONF_REG        (0x01)
#define TMP75_TEMP_AD_STEP	  (0.0625)

bool TMP75_setup(i2c_master_dev_handle_t device);
bool TMP75_getTemp(float *temp);

#endif // TMP110_H