#include "driver/i2c_master.h"
#include <unistd.h>
#include "esp_console.h"

#define I2C_TOOL_TIMEOUT_VALUE_MS (50u)

extern i2c_master_bus_handle_t tool_bus_handle;

static void register_i2cdetect(void);
static int do_i2cdetect_cmd(int argc, char **argv);

void i2cdetect_setup(void)
{
    register_i2cdetect();
}

static int do_i2cdetect_cmd(int argc, char **argv)
{
    uint8_t address;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            fflush(stdout);
            address = i + j;
            esp_err_t ret = i2c_master_probe(tool_bus_handle, address, I2C_TOOL_TIMEOUT_VALUE_MS);
            if (ret == ESP_OK) {
                printf("%02x ", address);
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }

    return 0;
}

static void register_i2cdetect(void)
{
    const esp_console_cmd_t i2cdetect_cmd = {
        .command = "i2cdetect",
        .help = "Scan I2C bus for devices",
        .hint = NULL,
        .func = &do_i2cdetect_cmd,
        .argtable = NULL
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&i2cdetect_cmd));
}
