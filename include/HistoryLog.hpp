#pragma once

#include "common.hpp"

namespace HistoryLog
{
    constexpr int BufferSize = 32;
    void init();
    void update( const MeasurementData& data);
    void clear();

    const MeasurementData* getSample(uint32_t index);
    int getBufferHead();
}