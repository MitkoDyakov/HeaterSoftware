#include "pdsetup.h"
#include "AP33772S.h"
#include "driver/i2c_master.h"
#include <unistd.h>
#include "esp_console.h"
#include "linenoise/linenoise.h"

#define WRITE_BUFF_LENGTH         6
#define I2C_TOOL_TIMEOUT_VALUE_MS (50u)

static uint8_t writeBuf[WRITE_BUFF_LENGTH];
SRC_SPRandEPR_PDO_Fields SRC_SPRandEPRpdoArray[MAX_PDO_ENTRIES] = {0}; 

bool PDisFine = false;

extern i2c_master_bus_handle_t tool_bus_handle;
i2c_master_dev_handle_t PD_handle = NULL;

// Declarations
void AP33772S_setup(void);
static void register_pdsetup(void);
static int do_pdsetup_cmd(int argc, char **argv);
void displayCurrentRange(unsigned int current_max);
void displayEPRVoltageMin(unsigned int current_max);
void displaySPRVoltageMin(unsigned int current_max);
int displayPDOInfo(int pdoIndex);
int currentMap(int current);
void setFixPDO(int pdoIndex);

// Definitions 
void pdsetupt_setup(void)
{
    i2c_device_config_t pd_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AP33772S_ADDRESS,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(tool_bus_handle, &pd_dev_cfg, &PD_handle));

    AP33772S_setup();
    register_pdsetup();
}

void AP33772S_setup(void)
{
    uint8_t reg_addr = CMD_SRCPDO;
    uint8_t rx_data[26] = {0};

    esp_err_t ret = i2c_master_transmit_receive(PD_handle, &reg_addr, 1, rx_data, 26, I2C_TOOL_TIMEOUT_VALUE_MS);

    if (ret == ESP_OK) {
        PDisFine = true;
        for (int i = 0; i < 26; i += 2) {
            // Store the bytes in the array of structs
            int pdoIndex = (i / 2);  // Calculate the PDO index
            SRC_SPRandEPRpdoArray[pdoIndex].byte0 = rx_data[i];
            SRC_SPRandEPRpdoArray[pdoIndex].byte1 = rx_data[i + 1];
        }
    }else{
        printf("Error: initializing USB PD\r\n");
        PDisFine = false;
    }
}

static void register_pdsetup(void)
{
    const esp_console_cmd_t pdsetup_cmd = {
        .command = "pdsetup",
        .help = "Setup PD over USB",
        .hint = NULL,
        .func = &do_pdsetup_cmd,
        .argtable = NULL
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&pdsetup_cmd));
}

static int do_pdsetup_cmd(int argc, char **argv)
{
  bool bingo = false;
  uint8_t numberOfPDO = 0;

  if(PDisFine)
  {
    printf("PD list: \r\n");
    for (int i = 0; i < 26; i += 2) {
        int pdoIndex = (i / 2);  // Calculate the PDO index
        if(displayPDOInfo(pdoIndex) == 0)
        {
          bingo = true;
          numberOfPDO++;
        }
    }

    if(!bingo)
    {
      printf("The list is empty :( \r\n");
      return 0;
    }

    printf("Select a fixed PDO 1-%u:", numberOfPDO);

    char *line = NULL;
    int number = 0;

    while (number < 1 || number > numberOfPDO)
    {
      line = linenoise(" ");

      if (line != NULL) 
      {
        number = atoi(line);
        if (number >= 1 && number <= numberOfPDO) 
        {
            printf("You entered: %u\r\n", number);
        } else {
            printf("Invalid number. Try again.\r\n");
        }
        linenoiseFree(line);
      }
      setFixPDO(number);
    }
  }else{
    printf("Reading AP33772S register went wrong. Nothing to show.\r\n");
  }

  return 0;
}

void displayCurrentRange(unsigned int current_max) {
  switch (current_max) {
    case 0:
      printf("0.00A ~ 1.24A (Less than)");
      break;
    case 1:
      printf("1.25A ~ 1.49A");
      break;
    case 2:
      printf("1.50A ~ 1.74A");
      break;
    case 3:
      printf("1.75A ~ 1.99A");
      break;
    case 4:
      printf("2.00A ~ 2.24A");
      break;
    case 5:
      printf("2.25A ~ 2.49A");
      break;
    case 6:
      printf("2.50A ~ 2.74A");
      break;
    case 7:
      printf("2.75A ~ 2.99A");
      break;
    case 8:
      printf("3.00A ~ 3.24A");
      break;
    case 9:
      printf("3.25A ~ 3.49A");
      break;
    case 10:
      printf("3.50A ~ 3.74A");
      break;
    case 11:
      printf("3.75A ~ 3.99A");
      break;
    case 12:
      printf("4.00A ~ 4.24A");
      break;
    case 13:
      printf("4.25A ~ 4.49A");
      break;
    case 14:
      printf("4.50A ~ 4.99A");
      break;
    case 15:
      printf("5.00A ~ (More than)");
      break;
    default:
      printf("Invalid value");
      break;
  }
}

void displayEPRVoltageMin(unsigned int current_max) {
  switch (current_max) {
    case 0:
      printf("Reserved");
      break;
    case 1:
      printf("15000mV~");
      break;
    case 2:
      printf("15000mV < VOLTAGE_MIN ≤ 20000mV ");
      break;
    case 3:
      printf("others");
      break;
    default:
      printf("Invalid value");
      break;
  }
}

void displaySPRVoltageMin(unsigned int current_max) {
  switch (current_max) {
    case 0:
      printf("Reserved");
      break;
    case 1:
      printf("3300mV~");
      break;
    case 2:
      printf("3300mV < VOLTAGE_MIN ≤ 5000mV ");
      break;
    case 3:
      printf("others");
      break;
    default:
      printf("Invalid value");
      break;
  }
}

int displayPDOInfo(int pdoIndex) {
  // Determine if it's SPR or EPR based on pdoIndex
  bool isEPR = (pdoIndex >= 7 && pdoIndex <= 12);  // 1-6 for SPR, 7-12 for EPR
  // Check if both bytes are zero
  if (SRC_SPRandEPRpdoArray[pdoIndex].byte0 == 0 && SRC_SPRandEPRpdoArray[pdoIndex].byte1 == 0) {
    return -1;  // If both bytes are zero, exit the function
  }
  
  // Print the PDO type and index
  printf(pdoIndex <= 6 ? "SRC_SPR_PDO" : " SRC_EPR_PDO");
  printf("%d", pdoIndex+1);
  printf(": ");
  
  // Now, the individual fields can be accessed through the union in the struct
  if (SRC_SPRandEPRpdoArray[pdoIndex].fixed.type == 0) {  // Fixed PDO
    // Print parsed values
    printf("Fixed PDO: ");
    printf("%d", SRC_SPRandEPRpdoArray[pdoIndex].fixed.voltage_max * (isEPR ? 200 : 100)); // Voltage in 200mV units for EPR, 100mV for SPR
    printf("mV ");
    displayCurrentRange(SRC_SPRandEPRpdoArray[pdoIndex].fixed.current_max);  // Assuming displayCurrentRange function is available
    printf("\r\n");
    return 0;
  } else {  // PPS or AVS PDO
    // Print parsed values
    printf(isEPR ? "AVS PDO: " : "PPS PDO: ");
    if (isEPR) {
      displayEPRVoltageMin(SRC_SPRandEPRpdoArray[pdoIndex].avs.voltage_min);  // Assuming displayVoltageMin function is available
    } else {
      displaySPRVoltageMin(SRC_SPRandEPRpdoArray[pdoIndex].pps.voltage_min);  // Assuming displayVoltageMin function is available
    }
    printf("%d", SRC_SPRandEPRpdoArray[pdoIndex].fixed.voltage_max * (isEPR ? 200 : 100)); // Maximum Voltage in 200mV units for EPR, 100mV for SPR
    printf("mV ");
    displayCurrentRange(SRC_SPRandEPRpdoArray[pdoIndex].fixed.current_max);  // Assuming displayCurrentRange function is available
    printf("\r\n");
    return 1;
  }
}

int currentMap(int current)
{
    // Check if the value is out of bounds
    if (current < 0 || current > 5000) {
        return -1; // Return -1 for invalid inputs
    }

    // If value is below 1250, return 0
    if (current < 1250) {
        return 0;
    }

    // Calculate the result for ranges above 1250
    return ((current - 1250) / 250) + 1;
}

void setFixPDO(int pdoIndex) 
{ 
  RDO_DATA_T rdoData;
  rdoData.byte0 = 0x00;
  rdoData.byte1 = 0x00;
  
  // For Fix voltage, only need to set PDO_INDEX and CURRENT_SEL
  // No need to change the selected voltage
  // handle the same in standard as well as EPR

  // PDO index need to be fixed type
  if(SRC_SPRandEPRpdoArray[pdoIndex-1].fixed.type == 0){
    rdoData.REQMSG_Fields.PDO_INDEX = pdoIndex;  // Index 1
    rdoData.REQMSG_Fields.CURRENT_SEL = SRC_SPRandEPRpdoArray[pdoIndex-1].fixed.current_max;
    // Note: For profile less than or equal to 3A power, CURRENT_SEL = 9 will not work.
    // rdoData.REQMSG_Fields.CURRENT_SEL = 9; 
    writeBuf[0] = CMD_PD_REQMSG;
    writeBuf[1] = rdoData.byte0;  // Store the upper 8 bits
    writeBuf[2] = rdoData.byte1;  // Store the lower 8 bits
    ESP_ERROR_CHECK(i2c_master_transmit(PD_handle, writeBuf, 3, -1));
  }
  return;
}

