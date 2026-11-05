
#include "common.hpp"
#include "HwConfig.hpp"
#include "RhTempSensor.hpp"

------------------------------------------------
namespace RhTempSensor
{
    void init();
    void update();
    uint16_t getRawRh();
    uint16_t getRawTemp();
    float getRh();
    float getTemp();
  
}
-----------------------------------------------
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
    uint8_t cmd = CMD_MEASURE_HIGH_PRECISION;

    if(I2C::write_bytes(HwConfig::Sht40Addr, &cmd, 1) != ESP_OK)
    {
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(10));

    if(I2C::read_bytes( HwConfig::Sht40Addr, data, sizeof(data)) != ESP_OK)
    {
        return;
    }

    TempRaw_ = (data[0] << 8) | data[1];
    HumidityRaw_ = (data[3] << 8) | data[4];

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

################################################################

esp_err_t I2C::write_bytes(uint8_t device_address, const uint8_t* data, size_t data_len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte( cmd, (device_address << 1) | I2C_MASTER_WRITE, true);

    if ((data != nullptr) && (data_len > 0))
    {
        i2c_master_write( cmd, data, data_len, true);
    }

    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin( I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));

    i2c_cmd_link_delete(cmd);

    return ret;
}


esp_err_t I2C::read_bytes(uint8_t device_address, uint8_t* data, size_t data_len)
{
    if ((data == nullptr) || (data_len == 0))
    {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte( cmd, (device_address << 1) | I2C_MASTER_READ, true);

    if (data_len > 1)
    {
        i2c_master_read( cmd, data, data_len - 1, I2C_MASTER_ACK);
    }

    i2c_master_read_byte( cmd, &data[data_len - 1],I2C_MASTER_NACK);

    i2c_master_stop(cmd);
    /* TODO check CRC */
  
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));

    i2c_cmd_link_delete(cmd);

    return ret;
}
