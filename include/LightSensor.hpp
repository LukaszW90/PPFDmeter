#pragma once
#include "common.hpp"

namespace LightSensor
{
  bool init();
  void update();

  int getLux();
  int getPpfd();
  uint8_t getProfile();
  int readRaw();
  void setProfile( Lux2PpfdProfile profile);
}
