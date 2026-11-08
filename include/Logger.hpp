#pragma once
#include "common.hpp"


namespace Logger
{
    
    void update( MeasurementData& data) ;
    loggerState getLoggerState();
    void start(const int hh, const int mm, uint64_t sampleOffset_sec = 5*60, int recordingTime_h = 24);

    void logData(int timestamp, MeasurementData& data);
    void init();
    void InitSpi();
    void SdCardInit();

}