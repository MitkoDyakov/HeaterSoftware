
i2c_master_dev_handle_t tmpDevice;

bool TMP75_setup(i2c_master_dev_handle_t device)
{
    tmpDevice = device;
    return true;
}

bool TMP75_getTemp(float *temp)
{
    uint8_t reg_addr = TMP75_TEMP_REG;   // 0x00 for TMP110/TMP75
    uint8_t rx_data[2] = {0};
    esp_err_t ret = i2c_master_transmit_receive(tmpDevice, &reg_addr, 1, rx_data, 2, I2C_TOOL_TIMEOUT_VALUE_MS);

    if (ret == ESP_OK) {
        // Combine bytes, then arithmetic right shift by 4 to get signed 12-bit (Q4)
        int16_t raw16 = (int16_t)((rx_data[0] << 8) | rx_data[1]);
        int16_t q4 = raw16 >> 4;                 // sign-extends correctly
        float temp_c = (float)q4 * 0.0625f;      // LSB = 0.0625 °C

        *temp = temp_c; // Return integer part only
        return true;
    } else {
        return false;
    }
}


