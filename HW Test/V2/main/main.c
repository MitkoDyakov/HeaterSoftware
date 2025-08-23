#include <stdio.h>
#include "esp_console.h"
#include "esp_vfs_fat.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include <unistd.h>

#include "00_display/display.h"
#include "01_i2cdetect/i2cdetect.h"
#include "02_inputdetect/inputdetect.h"
#include "03_wifidetect/wifidetect.h"
#include "04_pdsetup/pdsetup.h"
#include "05_temptest/temptest.h"
#include "06_soundtest/soundtest.h"
#include "07_backlighttest/backlighttest.h"
#include "08_heatertest/heatertest.h"
#include "09_fantest/fantest.h"
#include "10_PID/pid_test.h"

#define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"
#define CONFIG_HEATER_TEST_PERIOD (1000u)

static i2c_port_t i2c_port = I2C_NUM_0;
static gpio_num_t i2c_gpio_scl = 46;
static gpio_num_t i2c_gpio_sda = 9;
i2c_master_bus_handle_t tool_bus_handle;

static void initialize_filesystem(void)
{
  static wl_handle_t wl_handle;
  const esp_vfs_fat_mount_config_t mount_config = {
      .max_files = 4,
      .format_if_mount_failed = true
  };
  esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(MOUNT_PATH, "storage", &mount_config, &wl_handle);
  if (err != ESP_OK) {
      printf("PROBLEM: no fat\r\n");
      return;
  }
}

void app_main(void)
{
  // I2C SETUP
  i2c_master_bus_config_t i2c_bus_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = i2c_port,
      .scl_io_num = i2c_gpio_scl,
      .sda_io_num = i2c_gpio_sda,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &tool_bus_handle));

  // CONSOLE SETUP
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

  initialize_filesystem();
  repl_config.history_save_path = HISTORY_PATH;
  repl_config.prompt = "test-app>";
  
  // install console REPL environment
  #if CONFIG_ESP_CONSOLE_UART
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
  #elif CONFIG_ESP_CONSOLE_USB_CDC
    esp_console_dev_usb_cdc_config_t cdc_config = ESP_CONSOLE_DEV_CDC_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_cdc(&cdc_config, &repl_config, &repl));
  #elif CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
    esp_console_dev_usb_serial_jtag_config_t usbjtag_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&usbjtag_config, &repl_config, &repl));
  #endif

  printf(" ===================================================================\n");
  printf(" |              Steps to Use heater test app                        |\n");
  printf(" |                                                                  |\n");
  printf(" |  1.  Try 'help' to check all supported commands                  |\n");
  printf(" |  2.  Try 'i2cdetect' to scan devices on the i2c bus              |\n");
  printf(" |  3.  Try 'inputdetect' to see the inputs from the buttons        |\n");
  printf(" |  4.  Try 'wifidetect' to list all available wifi networks        |\n");
  printf(" |  5.  Try 'pdsetup' to setup PD over USB                          |\n");
  printf(" |  6.  Try 'temptest' to read the ambient temp sensor              |\n");
  printf(" |  7.  Try 'soundtest' to test the speaker                         |\n");
  printf(" |  8.  Try 'backlighttest' to set screen brightness from 0 to 100  |\n");
  printf(" |  9.  Try 'heatertest' to run a test on a heater                  |\n");
  printf(" |  10. Try 'fantest' to test the fan channels                      |\n");
  printf(" ===================================================================\n\n");

  i2cdetect_setup();
  // inputdetect_setup();
  wifidetect_setup();
  pdsetupt_setup();
  temptest_setup();
  soundtest_setup();
  backlighttest_setup();
  heatertest_setup();
  fantest_setup();
  // pidtest_setup();

  lcd_init();
  display_jpeg_espjpeg("/data/logoJ.jpg");

  // start console REPL
  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
