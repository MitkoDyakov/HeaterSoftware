#include "temptest.h"
#include "esp_console.h"
#include <unistd.h>
#include "driver/i2c_master.h"

#define TMP75_ADDRESS         (0x48)
#define TMP75_TEMP_REG        (0x00)
#define TMP75_CONF_REG        (0x01)
#define TMP75_TEMP_AD_STEP	  (0.0625)
#define I2C_TOOL_TIMEOUT_VALUE_MS (50u)

i2c_master_dev_handle_t temp_handle = NULL;

void TMP75_setup(void);
static int do_temptest_cmd(int argc, char **argv);
static void register_temptest(void);

extern i2c_master_bus_handle_t tool_bus_handle;

void temptest_setup(void){
    TMP75_setup();
    register_temptest();
}

void TMP75_setup(void)
{
    i2c_device_config_t temp_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TMP75_ADDRESS,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(tool_bus_handle, &temp_dev_cfg, &temp_handle));
}

static int do_temptest_cmd(int argc, char **argv)
{
  uint8_t reg_addr = TMP75_TEMP_REG;
  uint8_t rx_data[2] = {0};
  uint16_t TempSum;

  esp_err_t ret = i2c_master_transmit_receive(temp_handle, &reg_addr, 1, rx_data, 2, I2C_TOOL_TIMEOUT_VALUE_MS);

  if (ret == ESP_OK)
  {
    TempSum = (((rx_data[0] << 8) | rx_data[1]) >> 4);
    TempSum = TempSum * TMP75_TEMP_AD_STEP;
    printf("Ambient temperature: %d°C\r\n", TempSum);
  }else{
    printf("Error: cannot read the ambient temperature.\r\n");
  }

  return 0;
}

static void register_temptest(void)
{
  const esp_console_cmd_t temptest_cmd = {
      .command = "temptest",
      .help = "Read the ambient temperature sensor.",
      .hint = NULL,
      .func = &do_temptest_cmd,
      .argtable = NULL
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&temptest_cmd));
}
