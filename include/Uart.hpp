#pragma once

namespace Uart
{
    void init();
    bool available();
    char read();
    void write(char c);
    void print(const char* str);
    void printf(const char* format, ...);
    void println(const char* str);
}