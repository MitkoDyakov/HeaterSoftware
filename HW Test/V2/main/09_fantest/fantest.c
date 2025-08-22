#include "fantest.h"
#include "esp_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "argtable3/argtable3.h"
#include "driver/gpio.h"

#define ENABLE_12V_PIN            (14u)
#define FAN_ENABLE_CHANNEL_1_PIN  (4u)
#define FAN_ENABLE_CHANNEL_2_PIN  (16u)
#define CONFIG_HEATER_TEST_PERIOD (1000u)

static int do_fantest_cmd(int argc, char **argv);
static void register_fantest(void);

void fantest_setup(void)
{
  // 12V
  gpio_reset_pin(ENABLE_12V_PIN);
  gpio_set_direction(ENABLE_12V_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(ENABLE_12V_PIN, 0);

  // FAN Control
  gpio_reset_pin(FAN_ENABLE_CHANNEL_1_PIN);
  gpio_set_direction(FAN_ENABLE_CHANNEL_1_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(FAN_ENABLE_CHANNEL_1_PIN, 0);

  gpio_reset_pin(FAN_ENABLE_CHANNEL_2_PIN);
  gpio_set_direction(FAN_ENABLE_CHANNEL_2_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(FAN_ENABLE_CHANNEL_2_PIN, 0);

  register_fantest();
}

static struct {
  struct arg_int *channel;
  struct arg_int *timeout;    
  struct arg_end *end;
} fantest_args;

static void register_fantest(void)
{
    fantest_args.channel = arg_int1("c", "channel", "<channel>", "Set the channel of the test 1 or 2.");
    fantest_args.timeout = arg_int0("t", "timeout", "<timeout>", "Set the duration of the test in sec (max 30).");
    fantest_args.end = arg_end(1);
    const esp_console_cmd_t timeout_cmd = {
        .command = "fantest",
        .help = "Test if fans work.",
        .hint = NULL,
        .func = &do_fantest_cmd,
        .argtable = &fantest_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&timeout_cmd));
}

static int do_fantest_cmd(int argc, char **argv)
{
  int nerrors = arg_parse(argc, argv, (void **)&fantest_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, fantest_args.end, argv[0]);
    return 0;
  }

  int channel = fantest_args.channel->ival[0];

  if(channel != 1 && channel != 2)
  {
    printf("Error: channel must be 1 or 2.\r\n`");
    return 0;
  }

  uint8_t duration = 30;

  if (fantest_args.timeout->count) {
      duration = fantest_args.timeout->ival[0];
  }

  if (duration > 30) {
    printf("Warning: max timeout is 30 seconds.\r\n`");
    duration = 30;
  }

  if(duration < 1)
  {
    printf("Error: timeout must be greater than 0.\r\n`");
    return 0;
  }

  gpio_set_level(ENABLE_12V_PIN, 1);
  
  if(channel == 1)
  {
    printf("Fan channel 1 is on ");
    gpio_set_level(FAN_ENABLE_CHANNEL_1_PIN, 1);
  }else{
    printf("Fan channel 2 is on ");
    gpio_set_level(FAN_ENABLE_CHANNEL_2_PIN, 1);
  }

  for(int i=0; i < duration; i++)
  {
    printf(".");
    fflush(stdout);
    vTaskDelay(CONFIG_HEATER_TEST_PERIOD / portTICK_PERIOD_MS);
  }

  printf(" done.\r\n");

  gpio_set_level(ENABLE_12V_PIN, 0);
  gpio_set_level(FAN_ENABLE_CHANNEL_1_PIN, 0);
  gpio_set_level(FAN_ENABLE_CHANNEL_2_PIN, 0);

  return 0;
}