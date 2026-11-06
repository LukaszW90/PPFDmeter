#pragma once

#include "common.h"

namespace OledDisplay
{
    void init();
    void clearBuffer(void) ;
    void clearScreen(void) ;
    void Update();
    void Refresh();

    /* easter egg */
    void drawScreenSaver();
    void drawVictrebelAt(int target_x, int target_y,const uint8_t* sprite, uint8_t *buffer);
    void drawScreen(MeasurementData& data, ConfigData& config, StatusData& status);

    constexpr int DisplayWidth_px = 128; /*(128px)*/;
    constexpr int DisplayHeight_px = 64;
    constexpr int DisplayHeight = DisplayHeight_px/8; /*(64px height, uint8 for 8px)*/;
    constexpr int DisplayArraySize = DisplayWidth_px*DisplayHeight; /*(8*8*128)*/;

}