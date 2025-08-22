#include "inputdetect.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"

#define IS_BIT_SET(n,x)   (((n & (1 << x)) != 0) ? 1 : 0)

#define IO_EXPANDER_ADP5585_ADDRESS (0x34)
#define IO_EXPANDER_RST_PIN         (1u)
#define IO_EXPANDER_INT_PIN         (2u)

QueueHandle_t interputQueue;
volatile bool flagISR = 0;
bool oldValueBtnReg[9] = {false};

extern i2c_master_bus_handle_t tool_bus_handle;
i2c_master_dev_handle_t ioexpander_handle = NULL;

static int do_inputdetect_cmd(int argc, char **argv);
static void register_inputdetect(void);

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interputQueue, &pinNumber, NULL);
    flagISR = 1;
}

void inputdetect_setup(void)
{
    // GPIO SETUP
    gpio_reset_pin(IO_EXPANDER_RST_PIN);
    gpio_set_direction(IO_EXPANDER_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(IO_EXPANDER_RST_PIN, 1);

    gpio_reset_pin(IO_EXPANDER_INT_PIN);
    gpio_set_direction(IO_EXPANDER_INT_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(IO_EXPANDER_INT_PIN, GPIO_INTR_NEGEDGE);

    interputQueue = xQueueCreate(10, sizeof(int));
    gpio_install_isr_service(0);
    gpio_isr_handler_add(IO_EXPANDER_INT_PIN, gpio_interrupt_handler, (void *)IO_EXPANDER_INT_PIN);

    // I2C SETUP
    i2c_device_config_t ioexpander_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = IO_EXPANDER_ADP5585_ADDRESS,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(tool_bus_handle, &ioexpander_dev_cfg, &ioexpander_handle));

    uint8_t data_wr[3] = {0x00, 0x00, 0x00};

    // Set as inputs
    data_wr[0] = 0x27;
    data_wr[1] = 0x00;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    data_wr[0] = 0x28;
    data_wr[1] = 0x00;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    // Add debaunce
    data_wr[0] = 0x21;
    data_wr[1] = 0xE0;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    data_wr[0] = 0x22;
    data_wr[1] = 0xF0;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    // Set INT on falling edge
    data_wr[0] = 0x1B;
    data_wr[1] = 0x00;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    data_wr[0] = 0x1C;
    data_wr[1] = 0x00;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    // Eneble INT
    data_wr[0] = 0x1F;
    data_wr[1] = 0x1F;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    data_wr[0] = 0x20;
    data_wr[1] = 0x0F;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    // Clear INT every 50us + set oscilator
    data_wr[0] = 0x3B;
    data_wr[1] = 0xA2;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    // Enable INT 
    data_wr[0] = 0x3C;
    data_wr[1] = 0x02;
    ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_wr, 2, -1));

    // CONSOLE SETUP
    register_inputdetect();
}

static int do_inputdetect_cmd(int argc, char **argv)
{
  uint8_t data_write[3] = {0x00, 0x00, 0x00};
  uint8_t data_read[3] = {0x00, 0x00, 0x00};
  bool newValueBtnReg[9] = {false};
  uint8_t idx = 0;

  // Clear GPI_INT_STAT_A GPI_INT_STAT_B
  data_write[0] = 0x13;
  ESP_ERROR_CHECK(i2c_master_transmit_receive(ioexpander_handle, data_write, 1, data_read, 2, -1));

  // Clear the INT
  data_write[0] = 0x01;
  data_write[1] = 0x02;
  ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_write, 2, -1));

  xQueueReset(interputQueue);

  printf("Press ENTER to quit.\r\n");
  
  while (1) {
    // Prepare select() to check for input
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    // Non-blocking check for input
    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

    if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds))
    {
      // Input available, read it
      char *line = linenoise("");

      if (line != NULL) {
        printf("Exit input monitor.\r\n");
        linenoiseFree(line);
        break;
      }
    }

    for(idx=0; idx<9; idx++)
    {
      newValueBtnReg[idx] = false;
    }

    if(flagISR == 1)
    {
        flagISR = 0;

        // Clear GPI_INT_STAT_A GPI_INT_STAT_B
        data_write[0] = 0x13;
        ESP_ERROR_CHECK(i2c_master_transmit_receive(ioexpander_handle, data_write, 1, data_read, 2, -1));
     
        // Clear the INT
        data_write[0] = 0x01;
        data_write[1] = 0x02;
        ESP_ERROR_CHECK(i2c_master_transmit(ioexpander_handle, data_write, 2, -1));

        if(IS_BIT_SET(data_read[0], 0))
        {
          newValueBtnReg[0] = true;
        }

        if(IS_BIT_SET(data_read[0], 1))
        {
          newValueBtnReg[1] = true;
        }

        if(IS_BIT_SET(data_read[0], 2))
        {
          newValueBtnReg[2] = true;
        }

        if(IS_BIT_SET(data_read[0], 3))
        {
          newValueBtnReg[3] = true;
        }

        if(IS_BIT_SET(data_read[0], 4))
        {
          newValueBtnReg[4] = true;
        }

        if(IS_BIT_SET(data_read[1], 0))
        {
          newValueBtnReg[5] = true;
        }

        if(IS_BIT_SET(data_read[1], 1))
        {
          newValueBtnReg[6] = true;
        }
        if(IS_BIT_SET(data_read[1], 2))
        {
          newValueBtnReg[7] = true;
        }

        if(IS_BIT_SET(data_read[1], 3))
        {
          newValueBtnReg[8] = true;
        }
    }

    for(idx=0; idx<9; idx++)
    {
      if(oldValueBtnReg[idx] != newValueBtnReg[idx])
      {
        switch (idx) 
        {
          case 0:
              if(newValueBtnReg[idx])
              {
                  printf("BUTTON 1 pressed\n");
              }else{
                  printf("BUTTON 1 release\n");
              }
              break;
          case 1:
              if(newValueBtnReg[idx])
              {
                  printf("RIGHT pressed\n");
              }else{
                  printf("RIGHT release\n");
              }
              break;
          case 2:
              if(newValueBtnReg[idx])
              {
                  printf("LEFT pressed\n");
              }else{
                  printf("LEFT release\n");
              }
              break;
          case 3:
              if(newValueBtnReg[idx])
              {
                  printf("SELECT pressed\n");
              }else{
                  printf("SELECT release\n");
              }
              break;
          case 4:
              if(newValueBtnReg[idx])
              {
                  printf("DOWN pressed\n");
              }else{
                  printf("DOWN release\n");
              }
              break;
          case 5:
              if(newValueBtnReg[idx])
              {
                  printf("BUTTON 2 pressed\n");                        
              }else{
                  printf("BUTTON 2 release\n");
              }     
              break;
          case 6:
              if(newValueBtnReg[idx])
              {
                  printf("BUTTON 3 pressed\n");
              }else{
                  printf("BUTTON 3 release\n");
              }    
              break;
          case 7:
              if(newValueBtnReg[idx])
              {
                  printf("BUTTON 4 pressed\n");
              }else{
                  printf("BUTTON 4 release\n");
              }   
              break;
          case 8:
              if(newValueBtnReg[idx])
              {
                  printf("UP pressed\n");
              }else{
                  printf("UP release\n");
              }
              break;
          default:
            // code block
          }                  
        oldValueBtnReg[idx] = newValueBtnReg[idx];
      }
    }
  }

  return 0;
}

static void register_inputdetect(void)
{
    const esp_console_cmd_t input_cmd = {
        .command = "inputdetect",
        .help = "Lists user inputs.",
        .hint = NULL,
        .func = &do_inputdetect_cmd,
        .argtable = NULL
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&input_cmd));
}