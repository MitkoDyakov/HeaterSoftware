#include "AP33772S.h"
#include "driver/i2c_master.h"
#include <unistd.h>
#include "esp_log.h"

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
  ESP_LOGI("pd", "AP33772S setup start");
  AP33772S_UpdatePdoList();
  ESP_LOGI("pd", "AP33772S setup complete five=%d nine=%d fifteen=%d twenty=%d",
           pdo_reg.fiveV, pdo_reg.nineV, pdo_reg.fifteenV, pdo_reg.twentyV);
  return true;
}

void AP33772S_UpdatePdoList(void)
{
    uint8_t reg_addr = CMD_SRCPDO;
    uint8_t rx_data[26] = {0};

    esp_err_t ret = i2c_master_transmit_receive(pdDevice, &reg_addr, 1, rx_data, 26, I2C_TOOL_TIMEOUT_VALUE_MS);
  ESP_LOGI("pd", "Read SRCPDO ret=%d", (int)ret);
  if (ret == ESP_OK) {
        for (uint8_t i = 0; i < 26; i += 2) {
            // Store the bytes in the array of structs
            uint8_t pdoIndex = (i / 2);  // Calculate the PDO index
            SRC_SPRandEPRpdoArray[pdoIndex].byte0 = rx_data[i];
            SRC_SPRandEPRpdoArray[pdoIndex].byte1 = rx_data[i + 1];
        }
        parsePDOlist();
  } else {
    ESP_LOGW("pd", "Failed to read PDO list ret=%d", (int)ret);
    }
}

void parsePDOlist()
{
  ESP_LOGI("pd", "Parsing PDO list (MAX_PDO_ENTRIES=%d)...", MAX_PDO_ENTRIES);

  // Reset previous results
  pdo_reg = (pdo_reg_t){0};

  for (int i = 0; i < MAX_PDO_ENTRIES; i++) {
    uint8_t b0 = SRC_SPRandEPRpdoArray[i].byte0;
    uint8_t b1 = SRC_SPRandEPRpdoArray[i].byte1;

    if (b0 == 0 && b1 == 0) {
      // Empty slot, skip (most arrays will be sparse)
      continue;
    }

    // Reconstruct 16-bit raw for clarity
    uint16_t raw = ((uint16_t)b1 << 8) | b0; // device appears to send low byte then high byte

    uint8_t type = (raw >> 14) & 0x1; // Bit 14 (using our struct definition)
    uint8_t detect = (raw >> 15) & 0x1; // Bit 15
    if (!detect) {
      ESP_LOGD("pd", "PDO idx=%d raw=0x%04X not detected (detect=0)", i, raw);
      continue; // Not a valid PDO entry yet
    }

    if (type != 0) {
      // Only handling fixed supply for now
      ESP_LOGD("pd", "PDO idx=%d raw=0x%04X type=%u (non-fixed ignored)", i, raw, type);
      continue;
    }

    uint8_t voltage_code = raw & 0xFF; // bits 7:0
    uint8_t current_code = (raw >> 10) & 0xF; // bits 13:10 (CURRENT_MAX code)

    // Voltage encoding clarification:
    // Observed codes: 50 -> 5V, 90 -> 9V, 150 -> 15V, 200 -> 20V.
    // Therefore units are 100mV (code * 100 = mV).
    uint32_t millivolts = (uint32_t)voltage_code * 100U; // 100mV units
    // Current decoding per legacy pdsetup.c displayCurrentRange(): each code corresponds to a range.
    // We'll expose the upper bound of the advertised range as a representative "max" current.
    static const uint16_t current_code_upper_mA[16] = {
      1240, 1490, 1740, 1990, 2240, 2490, 2740, 2990,
      3240, 3490, 3740, 3990, 4240, 4490, 4990, 5000
    };
    uint32_t milliAmps = current_code_upper_mA[current_code];

  ESP_LOGI("pd", "PDO idx=%d raw=0x%04X mv=%lu mA~%lu codeV=%u codeI=%u", i, raw,
       (unsigned long)millivolts, (unsigned long)milliAmps, voltage_code, current_code);

    switch (millivolts) {
      case 5000:
        pdo_reg.fiveV = true;
        pdo_reg.fiveV_index = i + 1; // 1-based index for RDO
        pdo_reg.fiveV_max_current = (uint16_t)milliAmps;
        break;
      case 9000:
        pdo_reg.nineV = true;
        pdo_reg.nineV_index = i + 1;
        pdo_reg.nineV_max_current = (uint16_t)milliAmps;
        break;
      case 15000:
        pdo_reg.fifteenV = true;
        pdo_reg.fifteenV_index = i + 1;
        pdo_reg.fifteenV_max_current = (uint16_t)milliAmps;
        break;
      case 20000:
        pdo_reg.twentyV = true;
        pdo_reg.twentyV_index = i + 1;
        pdo_reg.twentyV_max_current = (uint16_t)milliAmps;
        break;
      default:
        ESP_LOGD("pd", "PDO idx=%d unsupported fixed voltage %lu mV (rawCode=%u)",
                 i, (unsigned long)millivolts, voltage_code);
        break;
    }
  }

  ESP_LOGI("pd", "PDO parse done five=%d(%umA) nine=%d(%umA) fifteen=%d(%umA) twenty=%d(%umA)",
           pdo_reg.fiveV, pdo_reg.fiveV_max_current,
           pdo_reg.nineV, pdo_reg.nineV_max_current,
           pdo_reg.fifteenV, pdo_reg.fifteenV_max_current,
           pdo_reg.twentyV, pdo_reg.twentyV_max_current);
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

void ap33772s_get_caps(ap33772s_caps_t *out) {
  if (!out) return;
  out->fiveV = pdo_reg.fiveV; out->cur5 = pdo_reg.fiveV ? (float)pdo_reg.fiveV_max_current / 1000.0f : 0.0f;
  out->nineV = pdo_reg.nineV; out->cur9 = pdo_reg.nineV ? (float)pdo_reg.nineV_max_current / 1000.0f : 0.0f;
  out->fifteenV = pdo_reg.fifteenV; out->cur15 = pdo_reg.fifteenV ? (float)pdo_reg.fifteenV_max_current / 1000.0f : 0.0f;
  out->twentyV = pdo_reg.twentyV; out->cur20 = pdo_reg.twentyV ? (float)pdo_reg.twentyV_max_current / 1000.0f : 0.0f;
}