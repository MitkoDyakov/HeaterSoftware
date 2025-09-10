#include "AP33772S.h"
#include "driver/i2c_master.h"
#include <unistd.h>

void AP33772S_UpdatePdoList(void);
void parsePDOlist();

#define WRITE_BUFF_LENGTH         3
#define I2C_TOOL_TIMEOUT_VALUE_MS (50u)
i2c_master_dev_handle_t pdDevice;

// Result structure for returning both ADC channels
typedef struct {
    bool fiveV;
    uint8_t fiveV_index; // 0=5V, 1=9V, 2=15V, 3=20V
    uint16_t fiveV_max_current; // in mA
    bool nineV;  
    uint8_t nineV_index; // 0=5V, 1=9V, 2=15V, 3=20V
    uint16_t nineV_max_current; // in mA
    bool fifteenV;
    uint8_t fifteenV_index; // 0=5V, 1=9V, 2=15V, 3=20V
    uint16_t fifteenV_max_current; // in mA
    bool twentyV;
    uint8_t twentyV_index; // 0=5V, 1=9V, 2=15V, 3=20V
    uint16_t twentyV_max_current; // in mA
} pdo_reg_t;

pdo_reg_t pdo_reg = {0};

SRC_SPRandEPR_PDO_Fields SRC_SPRandEPRpdoArray[MAX_PDO_ENTRIES] = {0}; 

bool AP33772S_setup(i2c_master_dev_handle_t devHandler)
{
  pdDevice = devHandler;
  AP33772S_UpdatePdoList();
  return true;
}

void AP33772S_UpdatePdoList(void)
{
    uint8_t reg_addr = CMD_SRCPDO;
    uint8_t rx_data[26] = {0};

    esp_err_t ret = i2c_master_transmit_receive(pdDevice, &reg_addr, 1, rx_data, 26, I2C_TOOL_TIMEOUT_VALUE_MS);

    if (ret == ESP_OK) {
        for (uint8_t i = 0; i < 26; i += 2) {
            // Store the bytes in the array of structs
            uint8_t pdoIndex = (i / 2);  // Calculate the PDO index
            SRC_SPRandEPRpdoArray[pdoIndex].byte0 = rx_data[i];
            SRC_SPRandEPRpdoArray[pdoIndex].byte1 = rx_data[i + 1];
        }
        parsePDOlist();
    }
}

void parsePDOlist()
{
    for (int i = 0; i < MAX_PDO_ENTRIES; i++) {
        if (SRC_SPRandEPRpdoArray[i].fixed.type == 0) { // Fixed Supply
            uint32_t voltage = SRC_SPRandEPRpdoArray[i].fixed.voltage_max * 50;

            switch (voltage) {
                case 5000:
                    pdo_reg.fiveV = true;
                    pdo_reg.fiveV_index = i+1;
                    break;
                case 9000:
                    pdo_reg.nineV = true;
                    pdo_reg.nineV_index = i+1;
                    break;
                case 15000:
                    pdo_reg.fifteenV = true;
                    pdo_reg.fifteenV_index = i+1;
                    break;
                case 20000:
                    pdo_reg.twentyV = true;
                    pdo_reg.twentyV_index = i+1;
                    break;
                default:
                    // Handle unexpected voltage values if necessary
                    break;
            }
        }
    }
}


bool setFixPDO(uint8_t voltage)
{
  static uint8_t writeBuf[WRITE_BUFF_LENGTH];
  RDO_DATA_T rdoData;
  rdoData.byte0 = 0x00;
  rdoData.byte1 = 0x00;
  rdoData.REQMSG_Fields.CURRENT_SEL = 0xf;
  rdoData.REQMSG_Fields.VOLTAGE_SEL = 0xff;

  switch(voltage){
    case 5:
      if(pdo_reg.fiveV){
        rdoData.REQMSG_Fields.PDO_INDEX = pdo_reg.fiveV_index;
      }
      break;
    case 9:
      if(pdo_reg.nineV){
        rdoData.REQMSG_Fields.PDO_INDEX = pdo_reg.nineV_index;
      }
      break;
    case 15:
      if(pdo_reg.fifteenV){
        rdoData.REQMSG_Fields.PDO_INDEX = pdo_reg.fifteenV_index; 
      }
      break;
    case 20:
      if(pdo_reg.twentyV){
        rdoData.REQMSG_Fields.PDO_INDEX = pdo_reg.twentyV_index; 
      }
      break;
    default:
      return false; // Invalid voltage
  }

  writeBuf[0] = CMD_PD_REQMSG;
  writeBuf[1] = rdoData.byte0;  // Store the upper 8 bits
  writeBuf[2] = rdoData.byte1;  // Store the lower 8 bits
  ESP_ERROR_CHECK(i2c_master_transmit(pdDevice, writeBuf, 3, -1));

  return true;
}