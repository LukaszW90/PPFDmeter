#include <algorithm>
#include <cstdint>

extern "C" {
    #include "driver/gpio.h"
    #include "esp_adc/adc_oneshot.h"
    #include "esp_adc/adc_cali.h"
    #include "esp_adc/adc_cali_scheme.h"
}

#include "HwConfig.hpp"
#include "HwInput.hpp"


/* ----------------- SWITCH ---------------------*/
/* init all digital inputs - HW switches*/
void initSwitch(gpio_num_t pin)
{
    gpio_config_t io_conf = {};

    /* BLE Switch */
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
}


void hwSwitchInput::init()
{
   // initSwitch(HwConfig::joyLeft_Switch);
}



bool hwSwitchInput::switchIsOn(gpio_num_t pin)
{
    return (gpio_get_level(pin) == 0);
}


/* ----------------- ADC ---------------------*/

static adc_oneshot_unit_handle_t adc1_handle = nullptr;
//static adc_cali_handle_t adc1_cali_handle = nullptr;

AnalogInput::AnalogInput(adc_channel_t channel) : channel_(channel)
{
    if (adc1_handle == nullptr)
    {
        adc_oneshot_unit_init_cfg_t init_cfg = {
            .unit_id  = ADC_UNIT_1,
            .clk_src  = ADC_DIGI_CLK_SRC_DEFAULT,
            .ulp_mode = ADC_ULP_MODE_DISABLE
        };

        ESP_ERROR_CHECK(
            adc_oneshot_new_unit(&init_cfg, &adc1_handle)
        );
    }

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc1_handle,
            channel_,
            &chan_cfg
        )
    );

 // ESP_LOGI("AnalogInput", "ADC initialized");
}


int AnalogInput::getAdcRaw() const
{
    int raw = 0;
    adc_oneshot_read(adc1_handle, channel_, &raw);
    return raw;
}

void AnalogInput::init()
{
    ;
}

void AnalogInput::update()
{
    const int sample = getAdcRaw();

    // filtr IIR 1/8
    avg_ += (sample - avg_) / 16;
}

int AnalogInput::getAdcfiltered() const
{
    return avg_;
}




int Joystick::getPositionX() const
{


    int value = adcX_.getAdcfiltered() - JOY_CALIBDATA::CENTER;

   // if(std::abs(value) < JOY_CALIBDATA::DEADBAND)
   // {
   //     return 0;
   // }

    return (value * 512) / 2048;
}

int Joystick::getPositionY() const
{


    int value = adcY_.getAdcfiltered() - JOY_CALIBDATA::CENTER;

   // if(std::abs(value) < JOY_CALIBDATA::DEADBAND)
   // {
   //     return 0;
   // }

    return (value * 512) / 2048;
}

Joystick::Joystick( AnalogInput& adcX,  AnalogInput& adcY, int center): adcX_(adcX), adcY_(adcY)
{
  ;
}

void Joystick::update()
{
    adcX_.update();
    adcY_.update();

}