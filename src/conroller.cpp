#include "common.hpp"

#include "HwInput.hpp"
#include "WifiManager.hpp"
#include "controller.hpp"
#include "OledDisplay.hpp"
#include "LightSensor.hpp"

bool ScreenSaverEnabled = false;

void controller::init()
{

}


void controller::periodic()
{
    MeasurementData data{};   
    data.lux = SENSOR_ERROR;
    data.ppfd = SENSOR_ERROR;
    data.humidity = SENSOR_ERROR;
    data.temperature = SENSOR_ERROR;

ConfigData config;

    LightSensor::update();
    data.lux = LightSensor::getLux();
    if (!ScreenSaverEnabled)
    {

        OledDisplay::drawScreen(data,config);
    }
    else
    {
#if 0

        // Wyrenderowanie klatki na podstawie aktualnych współrzędnych
        OledDisplay::drawVictrebelAt(posX, posY);
#endif
        OledDisplay::drawScreen(data,config);
        OledDisplay::drawScreenSaver();
    }

    OledDisplay::Refresh();

}

void controller::setScreensaverEnabled(bool enabled)
{
    ScreenSaverEnabled = enabled;
}
