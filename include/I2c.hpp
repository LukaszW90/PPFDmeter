#pragma once

#include "HwConfig.hpp"

namespace I2C
{
    struct DeviceInfo
    {
        uint8_t address;
        const char* name;
    };
    constexpr DeviceInfo KnownDevices[] =
    {
        { HwConfig::I2cAddr::Veml7700, "VEML7700" },
        { HwConfig::I2cAddr::Oled, "SSD1306 (OLED)"  },
        { HwConfig::I2cAddr::Sht40, "SHT40"    },
        { HwConfig::I2cAddr::Ds3231, "DS3231"   },
        { HwConfig::I2cAddr::Ds3231Eeprom, "AT24C32 (EEPROM)"  }
    };

    void init();
    int scan(int *devAddr);
    esp_err_t write_bytes(uint8_t device_address, uint8_t reg_address, const uint8_t* data, size_t data_len); 
    esp_err_t read_bytes(uint8_t device_address, uint8_t reg_address, uint8_t* data, size_t data_len);
    const char* GetDeviceName(uint8_t address);

    constexpr int devicesNmb = 6; 
}