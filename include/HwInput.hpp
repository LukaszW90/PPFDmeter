#pragma once

#include <cstdint>
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"     // nowy driver ADC
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "HwConfig.hpp"
/* -----------------ADC & Output HW---------------------*/
constexpr int MAX_PWM_DUTY = (1 << 13) - 1;   // 8191

void init_ADC();   // jeśli potrzebujesz globalnej inicjalizacji

namespace hwSwitchInput
{
    bool switchIsOn(gpio_num_t pin);
    void init();
};

/* ----------------- AnalogInput ---------------------*/

class AnalogInput
{
public:
    AnalogInput() = default;
    ~AnalogInput() = default;

    explicit AnalogInput(adc_channel_t channel);

    void update();
    void init();

    int getAdcRaw() const;
    int getAdcfiltered() const;

private:
    adc_channel_t channel_;   // nowy typ kanału

    int avg_{0};
    int currentLevel_{0};
    int currentPwmLevel_{0};
};

namespace JOY_CALIBDATA
{
  constexpr int DEADBAND = 30;
  constexpr int CENTER = 2048; //45 offset
}

class Joystick
{


public:

    ~Joystick() = default;
    explicit Joystick( AnalogInput& adcX,  AnalogInput& adcY, int center = 2048);
    int getPositionX() const;
    int getPositionY() const;

    void update();

private:
    AnalogInput& adcX_ ;
    AnalogInput& adcY_ ;
};