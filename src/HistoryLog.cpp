extern "C"
 {
    #include "esp_log.h"
    #include "esp_err.h"
    #include "freertos/FreeRTOS.h"
 }
 #include <inttypes.h>
#include "Timer.hpp"
#include "HistoryLog.hpp"

uint64_t LastSampleTimeSec{0};
constexpr int RecordingOffsetMin  = 5;
MeasurementData HistoryBuffer[HistoryLog::BufferSize];
int Head {0};

void  HistoryLog::init()
{
    HistoryLog::clear();
}
void HistoryLog::clear()
{
  memset(&HistoryBuffer[0],0,sizeof(HistoryBuffer));
  Head = 0;
  LastSampleTimeSec = Timer::getSysTime_sec() - Timer::hmsToSec(0,RecordingOffsetMin );
}
const MeasurementData* HistoryLog::getSample(uint32_t index)
{
  if(index >= BufferSize)
  {
      return nullptr;
  }
  return &HistoryBuffer[index];
}
int HistoryLog::getBufferHead()
{
    return Head;
}

void HistoryLog::update( const MeasurementData& data)
{
  uint64_t currentTime_sec = Timer::getSysTime_sec();

  if (currentTime_sec >= (LastSampleTimeSec + Timer::hmsToSec(0,RecordingOffsetMin )) )
  {
    LastSampleTimeSec = Timer::getSysTime_sec();
    HistoryBuffer[Head] = data;
    Head = (Head + 1) % BufferSize;

    printf("%" PRIu32 "\n", HistoryBuffer[Head].lux);
  }
  
}