#include "common.hpp"

namespace History
{
    constexpr int BufferSize = 32;
    void init();
    void update( const MeasurementData& data);
    void clear();

    const MeasurementData* getSample(uint32_t index);
    int getBufferHead();
}
###################################################################


#include "Timer.hpp"
#include "History.hpp"

uint64_t LastSampleTimeSec{0};
constexpr int RecordingOffsetMin  = 5;
MeasurementData HistoryBuffer[History::BufferSize];
int Head {0};

void  History::init()
{
  ;
}
void History::clear()
{
  memset(&HistoryBuffer[0],0,sizeof(HistoryBuffer));
  Head = 0;
  LastSampleTimeSec = Timer::getSysTime_sec();
}
const MeasurementData* History::getSample(uint32_t index)
{
  if(index >= BufferSize)
  {
      return nullptr;
  }
  return &HistoryBuffer[index];
}
int History::getBufferHead()
{
    return Head;
}

void History::update( const MeasurementData& data)
{
  uint64_t currentTime_sec = Timer::getSysTime_sec();
  if (currentTime_sec >= LastSampleTimeSec + Timer::hmsToSec(0,RecordingOffsetMin ) )
  {
    LastSampleTimeSec = Timer::getSysTime_sec();
    HistoryBuffer[Head] = data;
    Head = (Head + 1) % BufferSize;
  }
  
}
