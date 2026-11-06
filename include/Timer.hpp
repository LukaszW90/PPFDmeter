#pragma once

namespace Timer
{
    constexpr int SEC_PER_MIN = 60;
    constexpr int SEC_PER_HOUR = 60 * SEC_PER_MIN;

    void init();
    void SetTimeHms(int hour, int minute, int second);
    uint64_t getTime_sec();
    uint64_t getSysTime_ms();
    uint64_t getSysTime_sec();

    constexpr int hmsToSec( int hour, int minute, int second = 0)
    {
        return  hour * SEC_PER_HOUR +  minute * SEC_PER_MIN + second;
    }

    constexpr void secToHms(int sec, int& hour, int& minutes)
    {
        hour  = sec / SEC_PER_HOUR;   minutes = (sec % SEC_PER_HOUR) / SEC_PER_MIN;
    }

}


