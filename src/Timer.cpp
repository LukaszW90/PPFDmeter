#include <chrono>
#include <thread>
#include "esp_timer.h"

extern "C"
 {
    #include "esp_log.h"
    #include "esp_err.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    #include <cmath>
}


#include "HwConfig.hpp"
#include "Timer.hpp"

static int BaseClockTimeSec = Timer::hmsToSec(10,20); 
static int BaseSysTimeSec = 0 ; 

void Timer::init()
{
    ;/* nothing to do */
}

void Timer::SetTimeHms(int hour, int minute, int second)
{

   BaseClockTimeSec = hmsToSec( hour, minute, second );
   BaseSysTimeSec = Timer::getSysTime_sec();
}

uint64_t  Timer::getTime_sec()
{
  
   constexpr uint64_t DAY_SEC = 24ULL * 60ULL * 60ULL;
   /* Time what was set (sec) (from hour) - time when time was set (from systimer)+ current systemtimer */
   return ( BaseClockTimeSec + Timer::getSysTime_sec() - BaseSysTimeSec ) % DAY_SEC;
}


/* internal systimer in msec */
uint64_t Timer::getSysTime_ms()
{
    return esp_timer_get_time()/ 1000;
}

/* Uptime from ECU start till now in sec */ 
uint64_t  Timer::getSysTime_sec() 
{

    using namespace std::chrono;
    uint64_t us = esp_timer_get_time();
    /* conversion to seconds */
    uint64_t sec = us / 1'000'000ULL;
    return sec;
}