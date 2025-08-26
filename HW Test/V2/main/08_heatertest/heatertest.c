
#include "ADS7142.h"
#include "esp_console.h"
#include <unistd.h>
#include "driver/i2c_master.h"
#include "argtable3/argtable3.h"
#include "driver/gpio.h"
#include "math.h"

#define ADS7142_RDY_PIN           (6u)
#define ADS7142_ALERT_PIN         (5u)
#define HEATR_ENABLE_CHANNEL_1    (7u)
#define HEATR_ENABLE_CHANNEL_2    (15u)
#define CONFIG_HEATER_TEST_PERIOD (1000u)
#define VALUE_PER_BIT             (0.00005035477) 

i2c_master_dev_handle_t ADC_handle = NULL;
extern i2c_master_bus_handle_t tool_bus_handle;

void ADS7142_setup(void);
static void register_heatertest(void);
static int do_heatertest_cmd(int argc, char **argv);

void heatertest_setup(void)
{
    // GPIO SETUP
    gpio_reset_pin(HEATR_ENABLE_CHANNEL_1);
    gpio_set_direction(HEATR_ENABLE_CHANNEL_1, GPIO_MODE_OUTPUT);
    gpio_set_level(HEATR_ENABLE_CHANNEL_1, 0);

    gpio_reset_pin(HEATR_ENABLE_CHANNEL_2);
    gpio_set_direction(HEATR_ENABLE_CHANNEL_2, GPIO_MODE_OUTPUT);
    gpio_set_level(HEATR_ENABLE_CHANNEL_2, 0);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << ADS7142_RDY_PIN), // Bit mask for GPIO4
        .mode = GPIO_MODE_INPUT,                   // Set as input mode
        .pull_up_en = GPIO_PULLUP_DISABLE,         // Disable pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,     // Disable pull-down
        .intr_type = GPIO_INTR_DISABLE             // No interrupt
    };

    // Apply the configuration
    gpio_config(&io_conf);

    i2c_device_config_t adc_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ADS7142_I2C_ADDRESS,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(tool_bus_handle, &adc_dev_cfg, &ADC_handle));

    ADS7142_setup();

    register_heatertest();
}

void ADS7142_setup(void)
{
    uint8_t cmd[3] = {0x00};

    //Abort the present sequence
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_ABORT_SEQUENCE;
    cmd[2] = ADS7142_VAL_ABORT_SEQUENCE;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));

    //Perform Offset Calibration
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_OFFSET_CAL;
    cmd[2] = ADS7142_VAL_TRIG_OFFCAL;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));

    while(gpio_get_level(ADS7142_RDY_PIN));

    //end of calibration

    //Select the channel input configuration
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_CHANNEL_INPUT_CFG;
    cmd[2] = ADS7142_VAL_CHANNEL_INPUT_CFG_2_CHANNEL_SINGLE_ENDED;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));

    //   //Confirm the input channel configuration
    //   uint8_t channelconfig;
    //   cmd[0] = SINGLE_REG_READ;
    //   cmd[1] = ADS7142_REG_CHANNEL_INPUT_CFG;
    //   ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 2, -1));
    //   ESP_ERROR_CHECK(i2c_master_receive(ADC_handle, &channelconfig, 1, -1)); 

    // ADS7142SingleRegisterRead(, &channelconfig);

    //Select the operation mode of the device
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_OPMODE_SEL;
    cmd[2] = ADS7142_VAL_OPMODE_SEL_HIGH_PRECISION_MODE;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));

    //   //Confirm the operation mode selection
    //   uint8_t opmodeselconfig;
    //   cmd[0] = SINGLE_REG_READ;
    //   cmd[1] = ADS7142_REG_OPMODE_SEL;
    //   ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 2, -1));
    //   ESP_ERROR_CHECK(i2c_master_receive(ADC_handle, &opmodeselconfig, 1, -1)); 

    //Auto Sequence both channels 0 and 1
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_AUTO_SEQ_CHEN;
    cmd[2] = ADS7142_VAL_AUTO_SEQ_CHENAUTO_SEQ_CH0_CH1;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));

    //   //Confirm Auto Sequencing is enabled
    //   uint8_t autoseqchenconfig;
    //   cmd[0] = SINGLE_REG_READ;
    //   cmd[1] = ADS7142_REG_AUTO_SEQ_CHEN;
    //   ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 2, -1));
    //   ESP_ERROR_CHECK(i2c_master_receive(ADC_handle, &autoseqchenconfig, 1, -1)); 

    //Select the Low Power Oscillator or high speed oscillator
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_OSC_SEL;
    cmd[2] = ADS7142_VAL_OSC_SEL_HSZ_HSO;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));

    //   //Confirm the oscillator selection
    //   uint8_t oscselconfig;
    //   cmd[0] = SINGLE_REG_READ;
    //   cmd[1] = ADS7142_REG_OSC_SEL;
    //   ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 2, -1));
    //   ESP_ERROR_CHECK(i2c_master_receive(ADC_handle, &oscselconfig, 1, -1)); 

    //Set the minimum nCLK value for one conversion to maximize sampling speed
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_nCLK_SEL;
    cmd[2] = 21;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));

    //   //Confirm the nCLK selection
    //   uint8_t nCLKselconfig;
    //   cmd[0] = SINGLE_REG_READ;
    //   cmd[1] = ADS7142_REG_nCLK_SEL;
    //   ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 2, -1));
    //   ESP_ERROR_CHECK(i2c_master_receive(ADC_handle, &nCLKselconfig, 1, -1)); 

    //Enable the accumulator
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_ACC_EN;
    cmd[2] = ADS7142_VAL_ACC_EN;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));
}

double calculate_rntc(double Vout) {
    const double Vdd = 3.3;       // supply (V)
    const double R1  = 9100.0;    // ohms

    // Precomputed from R2=3200, R3=3200, R4=1500:
    // R23 = 1600,  A = (R23+R4)/R23 = 1.9375,  B = R4/R3 = 0.46875
    const double A = 1.9375;
    const double B = 0.46875;

    // C = R1/(Rntc + R1)
    double C = (Vout + B * Vdd) / (Vdd * A);

    if (C <= 0.0 || C >= 1.0) {
        return NAN;               // out-of-range Vout → invalid resistance
    }
    return R1 * (1.0 - C) / C;    // Rntc in ohms
}

double calculate_temperature_celsius(double Rntc) {
    const double A = 0.0008898765062;
    const double B = 0.0002511457735;
    const double C = 0.0000001932659254;

    if (Rntc <= 0.0) return -273.15;

    double lnR = log(Rntc);
    double inv_T = A + B * lnR + C * pow(lnR, 3);
    return (1.0 / inv_T) - 273.15;
}

static struct {
    struct arg_int *channel;
    struct arg_int *timeout;
    struct arg_end *end;
} heatertest_args;

static void register_heatertest(void)
{
    heatertest_args.channel = arg_int1("c", "channel", "<channel>", "Set the channel of the test 1 or 2.");
    heatertest_args.timeout = arg_int0("t", "timeout", "<timeout>", "Set the duration of the test in sec (max 10).");
    heatertest_args.end = arg_end(1);
    const esp_console_cmd_t heatertest_cmd = {
        .command = "heatertest",
        .help = "Run a test cycle on the selected heater.",
        .hint = NULL,
        .func = &do_heatertest_cmd,
        .argtable = &heatertest_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&heatertest_cmd));
}

static int do_heatertest_cmd(int argc, char **argv)
{
  uint8_t cmd[2] = {0x00};

  int nerrors = arg_parse(argc, argv, (void **)&heatertest_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, heatertest_args.end, argv[0]);
    return 0;
  }

  int channel = heatertest_args.channel->ival[0];

  if(channel != 1 && channel != 2)
  {
    printf("Error: channel must be 1 or 2.\r\n`");
    return 0;
  }

  uint8_t duration = 5;

  if (heatertest_args.timeout->count) {
      duration = heatertest_args.timeout->ival[0];
  }

  if (duration > 10) {
    printf("Warning: max duration is 10 seconds.\r\n`");
    duration = 10;
  }

  if(duration < 1)
  {
    printf("Error: duration must be greater than 0.\r\n`");
    return 0;
  }

  if(channel == 1)
  {
    printf("Heater channel 1 is on!\r\n");
    gpio_set_level(HEATR_ENABLE_CHANNEL_1, 1);
  }else{
    printf("Heater channel 2 is on!\r\n");
    gpio_set_level(HEATR_ENABLE_CHANNEL_2, 1);
  }

  for(int i=0; i < duration; i++)
  {
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_START_SEQUENCE;
    cmd[2] = ADS7142_VAL_START_SEQUENCE;
    ESP_ERROR_CHECK(i2c_master_transmit(ADC_handle, cmd, 3, -1));

    // call count
    while(gpio_get_level(ADS7142_RDY_PIN));

    //Read the MSB of Ch0 Accumulated Data after 16 accumulations are complete
    uint8_t accch0MSB = 0;
    cmd[0] = SINGLE_REG_READ;
    cmd[1] = ADS7142_REG_ACC_CH0_MSB;
    i2c_master_transmit_receive(ADC_handle, cmd, 2, &accch0MSB, 1, -1);

    //Read the LSB of Ch0 Accumulated Data after 16 accumulations are complete
    uint8_t accch0LSB = 0;
    cmd[0] = SINGLE_REG_READ;
    cmd[1] = ADS7142_REG_ACC_CH0_LSB;
    i2c_master_transmit_receive(ADC_handle, cmd, 2, &accch0LSB, 1, -1);

    //Read the MSB of Ch1 Accumulated Data after 16 accumulations are complete
    uint8_t accch1MSB = 0;
    cmd[0] = SINGLE_REG_READ;
    cmd[1] = ADS7142_REG_ACC_CH1_MSB;
    i2c_master_transmit_receive(ADC_handle, cmd, 2, &accch1MSB, 1, -1);

    //Read the LSB of Ch1 Accumulated Data after 16 accumulations are complete
    uint8_t accch1LSB = 0;
    cmd[0] = SINGLE_REG_READ;
    cmd[1] = ADS7142_REG_ACC_CH1_LSB;
    i2c_master_transmit_receive(ADC_handle, cmd, 2, &accch1LSB, 1, -1); 

    uint16_t ch0 = ((uint16_t)accch0MSB << 8) | accch0LSB;
    uint16_t ch1 = ((uint16_t)accch1MSB << 8) | accch1LSB;

    if(channel == 1)
    {      
      double voltage = VALUE_PER_BIT * (double)ch0;
      
      double Rntc = calculate_rntc(voltage);

      double tempC = calculate_temperature_celsius(Rntc);
      printf("ch1: %u - %f V - Rntc %f R - %.2f °C\r\n", ch0, voltage, Rntc, tempC);

    }else{
      double voltage = VALUE_PER_BIT * (double)ch1;
      
      double Rntc = calculate_rntc(voltage);

      double tempC = calculate_temperature_celsius(Rntc);
      printf("ch2: %u - %f V - Rntc %f R - %.2f °C\r\n", ch1, voltage, Rntc, tempC);


    }

    fflush(stdout);
    vTaskDelay(CONFIG_HEATER_TEST_PERIOD / portTICK_PERIOD_MS);
  }

  printf(" done.\r\n");
  gpio_set_level(HEATR_ENABLE_CHANNEL_1, 0);
  gpio_set_level(HEATR_ENABLE_CHANNEL_2, 0);

  return 0;
}

