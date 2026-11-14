#include "common.hpp"
#include "I2c.hpp"
#include "LightSensor.hpp"


/* VEML7700 */
constexpr uint8_t DEVICE_ADDR = 0x10;
constexpr uint8_t REG_ALS_CONF = 0x00;
constexpr uint8_t REG_ALS_DATA = 0x04;
constexpr float LUX_FACTOR = 0.0576f;

static float RawFiltered  = -1;
Lux2PpfdProfile currentProfile =  Lux2PpfdProfile::Led6500K;

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

Lux2PpfdProfile LightSensor::getProfile()
{
  return currentProfile;
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
