#include "common.hpp"

#include "HwInput.hpp"
#include "WifiManager.hpp"
#include "controller.hpp"
#include "OledDisplay.hpp"

bool ScreenSaverEnabled = true;

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


    if (!ScreenSaverEnabled)
    {

        OledDisplay::drawScreen(data);
    }
    else
    {
#if 0

        // Wyrenderowanie klatki na podstawie aktualnych współrzędnych
        OledDisplay::drawVictrebelAt(posX, posY);
#endif
        OledDisplay::drawScreen(data);
        OledDisplay::drawScreenSaver();
    }

    OledDisplay::Refresh();

}

void controller::setScreensaverEnabled(bool enabled)
{
    ScreenSaverEnabled = enabled;
}
