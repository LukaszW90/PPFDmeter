#pragma once

#include <chrono>

#if 0
    #define DEBUG
#endif

extern "C" 
{
    #include "esp_timer.h"
}

/* main loop timings */
#ifndef DEBUG
    constexpr int STEP_DELAY_MS = 75;          // normal mode tick
#else
    constexpr int STEP_DELAY_MS = 25;            // debug mode tick
#endif

namespace Timer
{
    constexpr int SEC_PER_MIN = 60;
    constexpr int SEC_PER_HOUR = 60 * SEC_PER_MIN;

    int getSysTime_sec();
    /* read rtc time from ds3231 in every 1 minute */
    void updateRtcTime();
    void init();

    int RTC_getTime_sec();
    bool RTC_SetHms(int hour, int minute, int second);
    bool RTC_GetHms(int& hour, int& minute, int& second);
    bool RTC_Ping();
    uint64_t getSysTime_ms();

    constexpr int hmsToSec( int hour, int minute, int second = 0)
    {
        return  hour * SEC_PER_HOUR +  minute * SEC_PER_MIN + second;
    }

    constexpr void secToHm(int sec, int& hour, int& minutes)
    {
        hour  = sec / SEC_PER_HOUR;   minutes = (sec % SEC_PER_HOUR) / SEC_PER_MIN;
    }

}


