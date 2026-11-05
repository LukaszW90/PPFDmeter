#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#include "common.hpp"
#include "HwConfig.hpp"
#include "I2c.hpp"
#include "RhTempSensor.hpp"


constexpr uint8_t CMD_MEASURE_HIGH_PRECISION = 0xFD;

static float Temperature_ = -1.0f;
static float Humidity_    = -1.0f;
static uint16_t TempRaw_ = 0xFFFF;
static uint16_t HumidityRaw_ = 0xFFFF;

void RhTempSensor::init()
{
  ; /* nothing to be done... */
}

void RhTempSensor::update()
{
    uint8_t data[6];
    memset(data, 0xAA, sizeof(data));
uint8_t cmd[] = {0x24, 0x00};

    if(I2C::write_bytes(HwConfig::I2cAddr::Sht30, &cmd[0], sizeof(cmd)) != ESP_OK)
    {
        TempRaw_ = -0.1f;
        HumidityRaw_ = -0.1f;
        return;
            }

    vTaskDelay(pdMS_TO_TICKS(50));

    if(I2C::read_bytes( HwConfig::I2cAddr::Sht30, data, sizeof(data) )!= ESP_OK)
    {
        TempRaw_ = -0.1f;
        HumidityRaw_ = -0.1f;
        return;

    }
    else
    {
        TempRaw_ = (data[0] << 8) | data[1];
        HumidityRaw_ = (data[3] << 8) | data[4];
    }

    Temperature_ = -45.0f + 175.0f *  TempRaw_ / 65535.0f;
    Humidity_ = -6.0f + 125.0f * HumidityRaw_ / 65535.0f;
    Humidity_ = std::min( 100.0f,std::max(0.0f, Humidity_));


}


uint16_t RhTempSensor::getRawRh()
{
    return HumidityRaw_;
}
uint16_t RhTempSensor::getRawTemp()
{
    return TempRaw_;
}
float RhTempSensor::getRh()
{
   return Humidity_;
}

float RhTempSensor:: getTemp()
{
    return Temperature_;
}


