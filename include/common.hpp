#pragma once


#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"

namespace StdError
{
    constexpr int OK = 0;
    constexpr int NOK = 0;
}

struct MeasurementData
{
    uint32_t lux;
    uint32_t ppfd;

    float temperature;
    float humidity;

    float dliSession;
    float dliTotal;
};

constexpr int SENSOR_ERROR = -1;
