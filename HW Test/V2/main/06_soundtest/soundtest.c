#include "soundtest.h"
#include "driver/ledc.h"
#include "esp_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SOUND_CONTROL_PIN         (21)
#define CONFIG_HEATER_TEST_PERIOD (1000u)

static int do_soundtest_cmd(int argc, char **argv);
static void register_soundtest(void);

ledc_channel_config_t sound_channel = {
    .channel    = LEDC_CHANNEL_1,
    .duty       = 0,
    .gpio_num   = SOUND_CONTROL_PIN,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .hpoint     = 0,
    .timer_sel  = LEDC_TIMER_1,
    .flags.output_invert = 0
};

void soundtest_setup(void){
    // PWM SETUP (backlight and sound)
    ledc_timer_config_t sound_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 4000,                      // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,    // timer mode
        .timer_num = LEDC_TIMER_1,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };

    // Set configuration of timer1 for high speed channels
    ledc_timer_config(&sound_timer);
    ledc_channel_config(&sound_channel);
    
    register_soundtest();
}

static void register_soundtest(void)
{
    const esp_console_cmd_t soundtest_cmd = {
        .command = "soundtest",
        .help = "Play a sound for 1 second.",
        .hint = NULL,
        .func = &do_soundtest_cmd,
        .argtable = NULL
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&soundtest_cmd));
}

static int do_soundtest_cmd(int argc, char **argv)
{
  printf("*beep*\r\n");
  
  ledc_set_duty(sound_channel.speed_mode, sound_channel.channel, 4095);
  ledc_update_duty(sound_channel.speed_mode, sound_channel.channel);

  vTaskDelay(CONFIG_HEATER_TEST_PERIOD / portTICK_PERIOD_MS);

  ledc_set_duty(sound_channel.speed_mode, sound_channel.channel, 0);
  ledc_update_duty(sound_channel.speed_mode, sound_channel.channel);
  return 0;
}