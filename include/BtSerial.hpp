#pragma once


namespace BtSerial {
    void begin(const char* name);
    int  available();
    char read();
    void write(char c);
    void print(const char* s);
    void printf(const char* format, ...);
    void println(const char* str);

    void init(const char* name);
    void deinit();

    bool enable();
    bool disable();
    void Periodic();
    void ResetTimeout();
    bool isBleActive();
}
