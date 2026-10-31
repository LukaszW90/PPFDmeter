#pragma once

namespace I2C
{
    void init();
    int scan(int *devAddr);
    constexpr int devicesNmb = 2; 
}