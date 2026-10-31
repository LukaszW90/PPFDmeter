extern "C"
{
    #include "driver/uart.h"

    #include <cstdarg>
    #include <cstdio>
}

#include "Uart.hpp"
#include "HwConfig.hpp"
#include <cstring>        // strlen
#include "driver/uart.h"  // uart_write_bytes

namespace
{
    constexpr uart_port_t UART_NUM = UART_NUM_0;
    constexpr int RX_BUF_SIZE = 1024;
}

void Uart::init()
{
    const uart_config_t uartConfig
    {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
#if ESP_IDF_VERSION_MAJOR >= 5
        .source_clk = UART_SCLK_DEFAULT,
#endif
    };

    uart_driver_install(
        UART_NUM,
        RX_BUF_SIZE,
        0,
        0,
        nullptr,
        0);

    uart_param_config(
        UART_NUM,
        &uartConfig);

    uart_set_pin(
        UART_NUM,
        HwConfig::UartTxPin,
        HwConfig::UartRxPin,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE);

    uart_flush(UART_NUM);

 //   Uart::print("\r\nUart ready\r\n");
}


bool Uart::available()
{
    size_t len = 0;

    uart_get_buffered_data_len(
        UART_NUM,
        &len);

    return len > 0;
}

char Uart::read()
{
    char c = '\0';

    uart_read_bytes(
        UART_NUM,
        reinterpret_cast<uint8_t*>(&c),
        1,
        0);

    return c;
}

void Uart::write(char c)
{
    uart_write_bytes(
        UART_NUM,
        &c,
        1);
}

void Uart::print(const char* str)
{
    uart_write_bytes(
        UART_NUM,
        str,
        strlen(str));
}

void Uart::printf(const char* format, ...)
{
    constexpr size_t BUFFER_SIZE = 128;

    char buffer[BUFFER_SIZE];

    va_list args;

    va_start(args, format);

    vsnprintf(
        buffer,
        BUFFER_SIZE,
        format,
        args);

    va_end(args);

    print(buffer);
}

void Uart::println(const char* str)
{
    Uart::print(str);
    Uart::print("\n\r");
}
