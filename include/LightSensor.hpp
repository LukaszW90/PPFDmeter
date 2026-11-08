#pragma once
#include "common.hpp"

namespace LightSensor
{
  bool init();
  void update();

  int getLux();
  int getPpfd();
  Lux2PpfdProfile getProfile();
  int readRaw();
  void setProfile( Lux2PpfdProfile profile);
}