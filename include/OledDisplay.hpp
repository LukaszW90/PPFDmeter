#pragma once

namespace OledDisplay
{
    void init();
    void printData(int x, int y, const char* text);
    void clearScreen(void) ;

    /* easter egg */
    void drawVictrebel();
    void drawVictrebelAt(int target_x, int target_y);
    
}