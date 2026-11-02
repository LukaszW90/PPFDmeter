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


if (!ScreenSaverEnabled)
{

    OledDisplay::drawScreen(0,0,0,0,0);
}
else
{
#if 0

        // Wyrenderowanie klatki na podstawie aktualnych współrzędnych
        OledDisplay::drawVictrebelAt(posX, posY);
#endif
    OledDisplay::drawScreen(0,0,0,0,0);
    OledDisplay::drawScreenSaver();
    }

    OledDisplay::Update();

}

void controller::setScreensaverEnabled(bool enabled)
{
    ScreenSaverEnabled = enabled;
}
