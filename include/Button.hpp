#pragma once

namespace Button
{
  enum class Event
  {  
      Released,
      OneClick,
      DoubleClick,
      LongPress
  };

  void init();
  void update();
  Event getEvent();
  bool isOn();
}