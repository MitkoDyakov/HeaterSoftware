#include "backlighttest.h"
#include "driver/ledc.h"
#include "esp_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "argtable3/argtable3.h"
#include <math.h>

#define BACKLIGHT_CONTROL_PIN   (47)
#define BACKLIGHT_DUTY_MAX   ((1 << 13) - 1)

static int do_backlighttest_cmd(int argc, char **argv);
static void register_backlighttest(void);

ledc_channel_config_t backlight_channel = {
    .channel    = LEDC_CHANNEL_0,
    .duty       = 0,
    .gpio_num   = BACKLIGHT_CONTROL_PIN,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .hpoint     = 0,
    .timer_sel  = LEDC_TIMER_2,
    .flags.output_invert = 0
};

void backlighttest_setup(void)
{
      ledc_timer_config_t backlight_timer = {
      .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
      .freq_hz = 500,                      // frequency of PWM signal
      .speed_mode = LEDC_LOW_SPEED_MODE,    // timer mode
      .timer_num = LEDC_TIMER_2,            // timer index
      .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };
    ledc_timer_config(&backlight_timer);
    ledc_channel_config(&backlight_channel);
    ledc_set_duty(backlight_channel.speed_mode, backlight_channel.channel, 4095);
    ledc_update_duty(backlight_channel.speed_mode, backlight_channel.channel);

    register_backlighttest();
}

static struct {
    struct arg_int *brightness;
    struct arg_end *end;
} backlighttest_args;

static void register_backlighttest(void)
{
    backlighttest_args.brightness = arg_int1("b", "brightness", "<brightness>", "Specify screen brightness between 0 and 100.");
    backlighttest_args.end = arg_end(1);
    const esp_console_cmd_t backlighttest_cmd = {
        .command = "backlighttest",
        .help = "Set the brightness of the screen.",
        .hint = NULL,
        .func = &do_backlighttest_cmd,
        .argtable = &backlighttest_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&backlighttest_cmd));
}

static int do_backlighttest_cmd(int argc, char **argv)
{
  int nerrors = arg_parse(argc, argv, (void **)&backlighttest_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, backlighttest_args.end, argv[0]);
    return 0;
  }

  int brightness = backlighttest_args.brightness->ival[0];
  
  if(brightness < 0 || brightness > 100)
  {
    printf("Error: brightness must be betweeen 0 and 100.\r\n`");
    return 0;
  }

  float level = brightness / 100.0f;
  float corrected = 0.02 + 0.98 * powf(level, 2.2);  // Gamma correction
  uint32_t dutyBooty = (uint32_t)(corrected * BACKLIGHT_DUTY_MAX);

  ledc_set_duty(backlight_channel.speed_mode, backlight_channel.channel, dutyBooty);
  ledc_update_duty(backlight_channel.speed_mode, backlight_channel.channel);

  return 0;
}