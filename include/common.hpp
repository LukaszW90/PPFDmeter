#pragma once

#include <stdio.h>
#include <iterator>

#include "esp_log.h"
#include "esp_err.h"

namespace StdError
{
    constexpr int OK = 0;
    constexpr int NOK = 1;
}

struct MeasurementData
{
    uint32_t lux;
    uint32_t ppfd;

    uint32_t temperature;
    uint32_t humidity;

    float dliSession;
    float dliTotal;
};

enum class Lux2PpfdProfile : uint8_t
{
    Led3200K = 0,
    Led4000K,
    Led6500K,
    Sun,

    Count
};

namespace Lux2ppfd_Calibration
{
    constexpr float Values[] =
    {
        0.0138f,  // warm Led3200K
        0.0145f,  // Led4000K
        0.0155f,  // cold Led6500K
        0.0185f   // Sun
    };
}
static_assert( static_cast<size_t>(Lux2PpfdProfile::Count) == std::size(Lux2ppfd_Calibration::Values));

struct ConfigData
{
//filename
//savinginterval
    Lux2PpfdProfile Profile =  Lux2PpfdProfile::Led3200K;
    uint32_t loggingInterval_s = 300;
    char fileName[32];
};


constexpr uint32_t SENSOR_ERROR = 0xFFFFFFFF;