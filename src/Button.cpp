#include "driver/gpio.h"
#include "HwConfig.hpp"
#include "Timer.hpp"
#include "Button.hpp"


namespace
{
  Button::Event currentEvent = Button::Event::Released;

  bool buttonPrev = false;
  bool longPressHandled = false;

  
  uint32_t pressTime = 0;
  uint32_t releaseTime = 0;
  
  uint8_t clicks = 0;
  
  constexpr uint32_t LongPressMs   = 2000;
  constexpr uint32_t DoubleClickMs = 500;

  uint32_t lastEdgeTime = 0;
}

void Button::init()
{
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << HwConfig::Button);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
}

void Button::update()
{
  bool pressed = isOn();
  
  if (pressed && !buttonPrev)
  {

      if((Timer::getSysTime_ms() - lastEdgeTime) > 30)
      {
        pressTime = Timer::getSysTime_ms();
        longPressHandled = false;

        lastEdgeTime = Timer::getSysTime_ms();
      }
  }

if (!pressed && buttonPrev)
{
    if (!longPressHandled)
    {
        clicks++;
        releaseTime = Timer::getSysTime_ms();
    }
}
  
if (pressed && !longPressHandled && (Timer::getSysTime_ms() - pressTime > LongPressMs))
    {
      currentEvent = Button::Event::LongPress;

      longPressHandled = true;
      clicks = 0;
    }
if (!pressed && clicks > 0 && (Timer::getSysTime_ms() - releaseTime > DoubleClickMs))
{
    if (clicks == 1)
    {
        currentEvent = Button::Event::OneClick;
    }
    else
    {
        currentEvent = Button::Event::DoubleClick;
    }

    clicks = 0;
}

  
  
  buttonPrev = pressed;
  
}
Button::Event Button::getEvent()
{
    auto ret = currentEvent;

    currentEvent = Button::Event::Released;

    return ret;
}

bool Button::isOn()
{    
    return (gpio_get_level(HwConfig::Button) == 0);
}