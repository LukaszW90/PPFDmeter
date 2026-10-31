#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"   

/**********************************
# WifiPilot_RC

GPIO 5  - joy 2 switch
GPIO 6  - switch 3
GPIO 7  - switch 4
GPIO 8  - adc battery input
GPIO 9  -   / (Boot) (led output 1/battery status)
GPIO 10 - led output 2 - connection status
GPIO 20 - Rx UART / NC
GPIO 21 - Tx UART / NC
////
5V
GND
3v3
GPIO 4 (Analog 4)  - joy 2 y 
GPIO 3 (Analog 3)  - joy 2 x
GPIO 2 (Analog 2)  - joy 1 switch
GPIO 1 (Analog 1)  - joy 1 y 
GPIO 0 (Analog 0)  - joy 1 x
**************************************/

namespace HwConfig
{

    /* RTC */
    constexpr gpio_num_t I2cSdaPin = GPIO_NUM_6;
    constexpr gpio_num_t I2cSclPin = GPIO_NUM_7;
    constexpr int I2cFreq = 40000;

    /* Uart  */
    constexpr gpio_num_t UartRxPin = GPIO_NUM_20;
    constexpr gpio_num_t UartTxPin = GPIO_NUM_21;
}