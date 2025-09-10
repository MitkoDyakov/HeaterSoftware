
#include "ADS7142.h"
#include "esp_console.h"
#include "driver/i2c_master.h"
#include "argtable3/argtable3.h"
#include "driver/gpio.h"
#include "math.h"
#include "pinout.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define VALUE_PER_BIT             (0.00005035477)
#define ADS7142_I2C_TIMEOUT_MS    (50)

double calculate_rntc(double Vout);
double calculate_temperature_celsius(double Rntc);

i2c_master_dev_handle_t ADCDevice;

// Signal when ADS7142 RDY pin goes low (conversion/calibration complete)
static SemaphoreHandle_t s_adc_rdy_sem = NULL;
static bool s_adc_isr_installed = false;

static void IRAM_ATTR adc_rdy_isr(void *arg)
{
    BaseType_t hp_task_woken = pdFALSE;
    if (s_adc_rdy_sem) {
        xSemaphoreGiveFromISR(s_adc_rdy_sem, &hp_task_woken);
    }

    if (hp_task_woken) {
        portYIELD_FROM_ISR();
    }
}

// Wait for RDY to go low once, enabling the GPIO interrupt only for the wait window
static inline void adc_wait_rdy_low(uint32_t timeout_ms)
{
    configASSERT(s_adc_rdy_sem != NULL);
    // Flush any pending give (binary semaphore: single take is enough)
    (void)xSemaphoreTake(s_adc_rdy_sem, 0);
    if (gpio_get_level(ADC_RDY) != 0) {
        gpio_intr_enable(ADC_RDY);
        (void)xSemaphoreTake(s_adc_rdy_sem, pdMS_TO_TICKS(timeout_ms));
        gpio_intr_disable(ADC_RDY);
    }
}

bool ADS7142_setup(i2c_master_dev_handle_t devHandler)
{
    ADCDevice = devHandler;

    uint8_t cmd[3] = {0x00};

    // Create semaphore and configure RDY pin interrupt once
    if (s_adc_rdy_sem == NULL) {
        s_adc_rdy_sem = xSemaphoreCreateBinary();
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << ADC_RDY,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    if (!s_adc_isr_installed) {
        esp_err_t isr_ret = gpio_install_isr_service(0);
        if (isr_ret == ESP_OK || isr_ret == ESP_ERR_INVALID_STATE) {
            // ESP_ERR_INVALID_STATE means service already installed
            s_adc_isr_installed = true;
        }
        
    ESP_ERROR_CHECK(gpio_isr_handler_add(ADC_RDY, adc_rdy_isr, NULL));
    // Keep disabled until explicitly waiting
    gpio_intr_disable(ADC_RDY);
    }

    //Abort the present sequence
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_ABORT_SEQUENCE;
    cmd[2] = ADS7142_VAL_ABORT_SEQUENCE;
    ESP_ERROR_CHECK(i2c_master_transmit(ADCDevice, cmd, 3, ADS7142_I2C_TIMEOUT_MS));

    //Perform Offset Calibration
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_OFFSET_CAL;
    cmd[2] = ADS7142_VAL_TRIG_OFFCAL;
    ESP_ERROR_CHECK(i2c_master_transmit(ADCDevice, cmd, 3, ADS7142_I2C_TIMEOUT_MS));

    // Wait for calibration done: RDY goes low (interrupt enabled only during wait)
    adc_wait_rdy_low(ADS7142_I2C_TIMEOUT_MS);

    //end of calibration

    //Select the channel input configuration
    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_CHANNEL_INPUT_CFG;
    cmd[2] = ADS7142_VAL_CHANNEL_INPUT_CFG_2_CHANNEL_SINGLE_ENDED;
    ESP_ERROR_CHECK(i2c_master_transmit(ADCDevice, cmd, 3, ADS7142_I2C_TIMEOUT_MS));

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
    ESP_ERROR_CHECK(i2c_master_transmit(ADCDevice, cmd, 3, ADS7142_I2C_TIMEOUT_MS));

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
    ESP_ERROR_CHECK(i2c_master_transmit(ADCDevice, cmd, 3, ADS7142_I2C_TIMEOUT_MS));

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
    ESP_ERROR_CHECK(i2c_master_transmit(ADCDevice, cmd, 3, ADS7142_I2C_TIMEOUT_MS));

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
    ESP_ERROR_CHECK(i2c_master_transmit(ADCDevice, cmd, 3, ADS7142_I2C_TIMEOUT_MS));

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
    ESP_ERROR_CHECK(i2c_master_transmit(ADCDevice, cmd, 3, ADS7142_I2C_TIMEOUT_MS));


    return true;
}

bool getTemperature(double *chan0, double *chan1)
{
    uint8_t cmd[3] = {0x00};

    double voltage = 0;
    double Rntc = 0; 

    cmd[0] = SINGLE_REG_WRITE;
    cmd[1] = ADS7142_REG_START_SEQUENCE;
    cmd[2] = ADS7142_VAL_START_SEQUENCE;
    ESP_ERROR_CHECK(i2c_master_transmit(ADCDevice, cmd, 3, ADS7142_I2C_TIMEOUT_MS));

    // Wait for conversion ready via semaphore (interrupt enabled only during wait)
    adc_wait_rdy_low(ADS7142_I2C_TIMEOUT_MS);

    if(NULL != chan0)
    {
        //Read the MSB of Ch0 Accumulated Data after 16 accumulations are complete
        uint8_t accch0MSB = 0;
        cmd[0] = SINGLE_REG_READ;
        cmd[1] = ADS7142_REG_ACC_CH0_MSB;
        i2c_master_transmit_receive(ADCDevice, cmd, 2, &accch0MSB, 1, ADS7142_I2C_TIMEOUT_MS);

        //Read the LSB of Ch0 Accumulated Data after 16 accumulations are complete
        uint8_t accch0LSB = 0;
        cmd[0] = SINGLE_REG_READ;
        cmd[1] = ADS7142_REG_ACC_CH0_LSB;
        i2c_master_transmit_receive(ADCDevice, cmd, 2, &accch0LSB, 1, ADS7142_I2C_TIMEOUT_MS);
        
        uint16_t ch0 = ((uint16_t)accch0MSB << 8) | accch0LSB;

        voltage = VALUE_PER_BIT * (double)ch0;
        Rntc = calculate_rntc(voltage);
        *chan0 = calculate_temperature_celsius(Rntc);
    }

    if(NULL != chan1)
    {
        //Read the MSB of Ch1 Accumulated Data after 16 accumulations are complete
        uint8_t accch1MSB = 0;
        cmd[0] = SINGLE_REG_READ;
        cmd[1] = ADS7142_REG_ACC_CH1_MSB;
        i2c_master_transmit_receive(ADCDevice, cmd, 2, &accch1MSB, 1, ADS7142_I2C_TIMEOUT_MS);

        //Read the LSB of Ch1 Accumulated Data after 16 accumulations are complete
        uint8_t accch1LSB = 0;
        cmd[0] = SINGLE_REG_READ;
        cmd[1] = ADS7142_REG_ACC_CH1_LSB;
        i2c_master_transmit_receive(ADCDevice, cmd, 2, &accch1LSB, 1, ADS7142_I2C_TIMEOUT_MS);

        uint16_t ch1 = ((uint16_t)accch1MSB << 8) | accch1LSB;

        voltage = VALUE_PER_BIT * (double)ch1;
        Rntc = calculate_rntc(voltage);
        *chan1 = calculate_temperature_celsius(Rntc);
    }

    return true;
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