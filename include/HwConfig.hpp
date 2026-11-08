#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"   

/**********************************
# WifiPilot_RC

GPIO 5  - 
GPIO 6  - SDA
GPIO 7  - SCL
GPIO 8  - SCK
GPIO 9  - MISO
GPIO 10 - MOSI
GPIO 20 - Rx UART / NC
GPIO 21 - Tx UART / NC
////
5V
GND
3v3
GPIO 4 (Analog 4)  - 
GPIO 3 (Analog 3)  - 
GPIO 2 (Analog 2)  - 
GPIO 1 (Analog 1)  -  
GPIO 0 (Analog 0)  - 
**************************************/

namespace HwConfig
{

    constexpr gpio_num_t Button = GPIO_NUM_0;

    /* I2C */
    constexpr gpio_num_t I2cSdaPin = GPIO_NUM_6;
    constexpr gpio_num_t I2cSclPin = GPIO_NUM_7;
    constexpr int I2cFreq = 100000;

    /* SPI */
    constexpr gpio_num_t SpiCsPin = GPIO_NUM_1;
    constexpr gpio_num_t SpiSckPin = GPIO_NUM_8;
    constexpr gpio_num_t SpiMosiPin = GPIO_NUM_9;
    constexpr gpio_num_t SpiMisoPin = GPIO_NUM_10;

    /* Uart  */
    constexpr gpio_num_t UartRxPin = GPIO_NUM_20;
    constexpr gpio_num_t UartTxPin = GPIO_NUM_21;



    /* I2C devicesAddr */
    namespace I2cAddr
    {
        constexpr uint8_t Oled         = 0x3C;
        constexpr uint8_t Veml7700     = 0x10;
        constexpr uint8_t Sht30        = 0x44;
        constexpr uint8_t Ds3231       = 0x68;
        constexpr uint8_t Ds3231Eeprom = 0x57;
    }
}