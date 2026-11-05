#pragma once

namespace RhTempSensor
{
    void init();
    void update();
    uint16_t getRawRh();
    uint16_t getRawTemp();
    float getRh();
    float getTemp();
  
}