#pragma once

namespace I2C
{
    void init();
    int scan(int *devAddr);
    esp_err_t write_bytes(uint8_t device_address, uint8_t reg_address, const uint8_t* data, size_t data_len); 
    constexpr int devicesNmb = 2; 
}