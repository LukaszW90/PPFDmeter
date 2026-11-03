extern "C"
 {
    #include "driver/i2c.h"
    #include "esp_log.h"
    #include "esp_err.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    #include "esp_timer.h"

    #include <cmath>
}

#include "HwConfig.hpp"
#include "I2c.hpp"

constexpr i2c_port_t I2C_MASTER_NUM = I2C_NUM_0;

void I2C::init()
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = HwConfig::I2cSdaPin;
    conf.scl_io_num = HwConfig::I2cSclPin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = HwConfig::I2cFreq;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

/* return number of found I2C devices and device Addres*/
int I2C::scan(int *devAddr)
{
    int devices = 0;

    for (int addr = 1; addr < 127; addr++)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        // wysyłamy adres WRITE i wymagamy ACK
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);

        i2c_master_stop(cmd);

        esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(cmd);

        // DS3231 = 0x68
       // if (addr == OLED_ADDR)
       if (ESP_OK == err)
        {
              devAddr[devices] = addr;//OLED_ADDR;
              devices++;
        //    break; // nie ma sensu skanować dalej
        }
    }

    return devices;
}

esp_err_t I2C::write_bytes(uint8_t device_address, uint8_t reg_address, const uint8_t* data, size_t data_len) 
{
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        // Adres urządzenia z bitem zapisu
        i2c_master_write_byte(cmd, (device_address << 1) | I2C_MASTER_WRITE, true);
        // Adres rejestru docelowego (lub bajt kontrolny)
        i2c_master_write_byte(cmd, reg_address, true);
        // Faktyczne dane
        i2c_master_write(cmd, data, data_len, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);
        return ret;
    }


/*-----------------------------------------------------------------------*/
struct I2C::DeviceInfo
{
    uint8_t address;
    const char* name;
};

constexpr I2C::DeviceInfo KnownDevices[] =
{
    { HwConfig::I2cAddr::Veml7700, "VEML7700" },
    { HwConfig::I2cAddr::Oled, "SSD1306 (OLED)"  },
    { HwConfig::I2cAddr::Sht40r, "SHT40"    },
    { HwConfig::I2cAddr::Ds3231, "DS3231"   },
    { HwConfig::I2cAddr::Ds3231Eeprom, "AT24C32 (EEPROM)"  }
};

const char* I2C::GetDeviceName(uint8_t address)
{
    for(const auto& dev :  I2C::KnownDevices)
    {
        if(dev.address == address)
        {
            return dev.name;
        }
    }

    return "Unknown";
}
