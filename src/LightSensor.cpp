#include "common.hpp"
#include "I2C.hpp"
#include "LightSensor.h"


/* VEML7700 */
constexpr uint8_t DEVICE_ADDR = 0x10;
constexpr uint8_t REG_ALS_CONF = 0x00;
constexpr uint8_t REG_ALS_DATA = 0x04;
constexpr float LUX_FACTOR = 0.0576f;

static float RawFiltered  = -1f;
Lux2PpfdProfile currentProfile =  Lux2PpfdProfile::Led4000K;

bool LightSensor::init()
{
    uint8_t cfg[2] =
    {
        0x00,
        0x00
    };

    return (  I2C::write_bytes( DEVICE_ADDR, REG_ALS_CONF, cfg,sizeof(cfg)) == ESP_OK );
}

void LightSensor::update()
{
   int raw = readRaw();
   if(raw < 0)
  {
      RawFiltered = -1;
      return;
  }
  
  if(RawFiltered < 0 )
  {
      RawFiltered = raw;
  }
  else
  {
      RawFiltered = ( RawFiltered * 0.9f + raw * 0.1f);
  }
  
}
int LightSensor::getLux()
{
  if(RawFiltered < 0)
  {
        return SENSOR_ERROR;
  }
  return RawFiltered * LUX_FACTOR;
}
int LightSensor::getPpfd()
{
   int lux = getLux();

    if(lux < 0)
    {
        return SENSOR_ERROR;
    }
    return static_cast<int>( lux * Lux2ppfd_Calibration::Values[ static_cast<uint8_t>(currentProfile) ]);
}

uint8_t LightSensor::getProfile()
{
  return static_cast<uint8_t>(currentProfile);
}

int LightSensor::readRaw(void)
{
   uint8_t data[2];

    if(I2C::read_bytes( DEVICE_ADDR, REG_ALS_DATA, data, 2) != ESP_OK)
    {
        return SENSOR_ERROR;
    }

    uint16_t raw = data[0] | (data[1] << 8);

    return raw;
}
void LightSensor::setProfile( Lux2PpfdProfile profile)
{
  currentProfile = profile;
}
/////////////////////////
esp_err_t I2C::read_bytes(uint8_t device_address, uint8_t reg_address, uint8_t* data, size_t data_len)
{
    if ((data == nullptr) || (data_len == 0))
    {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Select register
    i2c_master_start(cmd);
    i2c_master_write_byte( cmd, (device_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte( cmd, reg_address, true);

    // Repeated START for read operation
    i2c_master_start(cmd);

    i2c_master_write_byte( cmd, (device_address << 1) | I2C_MASTER_READ, true);

    if (data_len > 1)
    {
        i2c_master_read( cmd, data, data_len - 1, I2C_MASTER_ACK);
    }

    i2c_master_read_byte( cmd, &data[data_len - 1], I2C_MASTER_NACK);

    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin( I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));

    i2c_cmd_link_delete(cmd);

    return ret;
}
