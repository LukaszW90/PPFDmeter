#pragma once

namespace OledDisplay
{
    void init();
    void printData(int x, int y, const char* text);
    void clearBuffer(void) ;
    void clearScreen(void) ;

    /* easter egg */
    void drawVictrebel();
    void drawVictrebelAt(int target_x, int target_y);
    void drawScreen(const int lux, const int ppfd, const int temp, const int humid, const int battery);

    constexpr int DisplayWidth_px = 128; /*(128px)*/;
    constexpr int DisplayHeight = 64/8; /*(64px height, uint8 for 8px)*/;
    constexpr int DisplaySize_px = DisplayWidth_px*DisplayHeight; /*(8*8*128)*/;

}