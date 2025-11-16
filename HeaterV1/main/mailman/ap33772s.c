#include "AP33772S.h"
#include "driver/i2c_master.h"
#include <unistd.h>
#include "esp_log.h"

void AP33772S_UpdatePdoList(void);
void parsePDOlist();

#define WRITE_BUFF_LENGTH         3
#define I2C_TOOL_TIMEOUT_VALUE_MS (50u)
i2c_master_dev_handle_t pdDevice;

// Generic fixed PDO entry (extensible for additional voltages or types later)
// NOTE: Legacy pdo_reg_t removed; higher layers now derive availability via ap33772s_get_caps()
// and internal snapshot (g_caps_snapshot).
typedef struct {
  uint16_t mv;        // Millivolts (5000, 9000, ...)
  uint16_t max_mA;    // Representative upper bound current
  uint8_t  pdo_pos;   // 1-based position in source capability list (0 = absent)
  uint8_t  type;      // 0=fixed, others reserved
  uint16_t raw16;     // Original 16-bit field for diagnostics
} ap33772s_fixed_pdo_t;

#define AP33772S_MAX_FIXED_TRACK 8
typedef struct {
  ap33772s_fixed_pdo_t entries[AP33772S_MAX_FIXED_TRACK];
  uint8_t count; // number of valid entries
} ap33772s_caps_snapshot_t;

static ap33772s_caps_snapshot_t g_caps_snapshot = {0}; // updated on AP33772S_UpdatePdoList()

// Helper lookups for canonical voltages (5/9/15/20V) to preserve existing outward behavior
static const ap33772s_fixed_pdo_t* find_voltage(uint16_t mv){
  for(uint8_t i=0;i<g_caps_snapshot.count;i++){
    if(g_caps_snapshot.entries[i].mv == mv) return &g_caps_snapshot.entries[i];
  }
  return NULL;
}

SRC_SPRandEPR_PDO_Fields SRC_SPRandEPRpdoArray[MAX_PDO_ENTRIES] = {0}; 

bool AP33772S_setup(i2c_master_dev_handle_t devHandler)
{
  pdDevice = devHandler;
  ESP_LOGI("pd", "AP33772S setup start");
  AP33772S_UpdatePdoList();
  const ap33772s_fixed_pdo_t *p5 = find_voltage(5000);
  const ap33772s_fixed_pdo_t *p9 = find_voltage(9000);
  const ap33772s_fixed_pdo_t *p15 = find_voltage(15000);
  const ap33772s_fixed_pdo_t *p20 = find_voltage(20000);
  ESP_LOGI("pd", "AP33772S setup complete five=%d nine=%d fifteen=%d twenty=%d (tracked=%u)",
           p5?1:0, p9?1:0, p15?1:0, p20?1:0, g_caps_snapshot.count);
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

  // Reset previous results (snapshot)
  g_caps_snapshot = (ap33772s_caps_snapshot_t){0};

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

    // Add to generic snapshot if room
    if (g_caps_snapshot.count < AP33772S_MAX_FIXED_TRACK) {
      ap33772s_fixed_pdo_t *slot = &g_caps_snapshot.entries[g_caps_snapshot.count++];
      slot->mv = (uint16_t)millivolts;
      slot->max_mA = (uint16_t)milliAmps;
      slot->pdo_pos = (uint8_t)(i + 1);
      slot->type = 0; // fixed
      slot->raw16 = raw;
    }

    // Non-canonical voltages are simply skipped for legacy view; still present in snapshot
  }

  const ap33772s_fixed_pdo_t *p5 = find_voltage(5000);
  const ap33772s_fixed_pdo_t *p9 = find_voltage(9000);
  const ap33772s_fixed_pdo_t *p15 = find_voltage(15000);
  const ap33772s_fixed_pdo_t *p20 = find_voltage(20000);
  ESP_LOGI("pd", "PDO parse done fixedCaps=%u five=%d(%u) nine=%d(%u) fifteen=%d(%u) twenty=%d(%u)",
           g_caps_snapshot.count,
           p5?1:0, p5? p5->max_mA:0,
           p9?1:0, p9? p9->max_mA:0,
           p15?1:0, p15? p15->max_mA:0,
           p20?1:0, p20? p20->max_mA:0);
}


bool setFixPDO(uint8_t voltage)
{
  uint16_t target_mv = 0;
  switch(voltage){
    case 5: target_mv = 5000; break;
    case 9: target_mv = 9000; break;
    case 15: target_mv = 15000; break;
    case 20: target_mv = 20000; break;
    default: return false; // unsupported voltage request
  }
  const ap33772s_fixed_pdo_t *p = find_voltage(target_mv);
  if(!p || p->pdo_pos == 0){
    ESP_LOGW("pd", "Requested %uV PDO not available", (unsigned)voltage);
    return false;
  }
  static uint8_t writeBuf[WRITE_BUFF_LENGTH];
  RDO_DATA_T rdoData = {0};
  rdoData.REQMSG_Fields.CURRENT_SEL = 0xf; // max
  rdoData.REQMSG_Fields.VOLTAGE_SEL = 0xff; // auto / as per device semantics
  rdoData.REQMSG_Fields.PDO_INDEX = p->pdo_pos; // 1-based
  writeBuf[0] = CMD_PD_REQMSG;
  writeBuf[1] = rdoData.byte0;
  writeBuf[2] = rdoData.byte1;
  ESP_ERROR_CHECK(i2c_master_transmit(pdDevice, writeBuf, 3, -1));
  ESP_LOGI("pd", "Requested fixed PDO %uV (index=%u, max=%umA)", (unsigned)voltage, p->pdo_pos, p->max_mA);
  return true;
}

void ap33772s_get_caps(ap33772s_caps_t *out) {
  if (!out) return;
  const ap33772s_fixed_pdo_t *p5 = find_voltage(5000);
  const ap33772s_fixed_pdo_t *p9 = find_voltage(9000);
  const ap33772s_fixed_pdo_t *p15 = find_voltage(15000);
  const ap33772s_fixed_pdo_t *p20 = find_voltage(20000);
  out->fiveV = p5 != NULL; out->cur5 = p5 ? (float)p5->max_mA / 1000.0f : 0.0f;
  out->nineV = p9 != NULL; out->cur9 = p9 ? (float)p9->max_mA / 1000.0f : 0.0f;
  out->fifteenV = p15 != NULL; out->cur15 = p15 ? (float)p15->max_mA / 1000.0f : 0.0f;
  out->twentyV = p20 != NULL; out->cur20 = p20 ? (float)p20->max_mA / 1000.0f : 0.0f;
}

// OPTIONAL: helper to find highest current PDO at/above a target millivoltage (not yet part of public API)
static const ap33772s_fixed_pdo_t* find_best_fixed(uint16_t target_mv){
  const ap33772s_fixed_pdo_t *best = NULL;
  for (uint8_t i = 0; i < g_caps_snapshot.count; ++i){
    const ap33772s_fixed_pdo_t *e = &g_caps_snapshot.entries[i];
    if (e->mv < target_mv) continue;
    if (!best || e->max_mA > best->max_mA) best = e;
  }
  return best;
}

uint16_t AP33772S_getCurrent()
{
  // 24mA/LSB
  uint8_t reg_addr = CMD_CURRENT;
  uint8_t rx_data = 0;
  esp_err_t ret = i2c_master_transmit_receive(pdDevice, &reg_addr, 1, &rx_data, 1, I2C_TOOL_TIMEOUT_VALUE_MS);
  return (uint16_t)(rx_data * 24);
}

uint8_t APS33772S_getTemperature()
{
  // 1C/LSB
  uint8_t reg_addr = CMD_TEMP;
  uint8_t rx_data = 0;
  esp_err_t ret = i2c_master_transmit_receive(pdDevice, &reg_addr, 1, &rx_data, 1, I2C_TOOL_TIMEOUT_VALUE_MS);
  return rx_data;
}