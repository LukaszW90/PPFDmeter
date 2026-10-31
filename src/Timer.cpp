#include <chrono>
#include <thread>

#if 0
extern "C"
 {
    #include "driver/i2c.h"
    #include "esp_log.h"
    #include "esp_err.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    #include "esp_timer.h"

    #include <cmath>
}
#include "Uart.hpp"
#include "HwConfig.hpp"
#include "Timer.hpp"

constexpr i2c_port_t I2C_MASTER_NUM = I2C_NUM_0;
constexpr int DS3231_ADDR = 0x68;

static uint8_t bcd2dec(uint8_t val) { return (val >> 4) * 10 + (val & 0x0F); }
static uint8_t dec2bcd(uint8_t val) { return ((val / 10) << 4) | (val % 10); }

static int TimeSec = Timer::hmsToSec(10,20); /* init value - 10:20 - override if external RTC DS 3232 is available */
/* status of connection (and prev connection) of external RTC module */
static bool DS3231Available = false; 
static bool DS3231PrevAvailableStatus = false; 

static int LastRtcSyncSystime_sec = 0; /* systick time of last external RTC connection - used to calculate time after connection lost */ 


void I2C::init()
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = HwConfig::I2cSdaPin;
    conf.scl_io_num = HwConfig::I2cSclPin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = HwConfig::rtcI2cFreq;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

/* return number of found I2C devices and device Addres*/
int I2C::scan(int &devAddr)
{
    int devices = 0;

    for (int addr = 1; addr < 127; addr++)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        // wysyłamy adres WRITE i wymagamy ACK
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);

        i2c_master_stop(cmd);

        esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(cmd);

        // DS3231 = 0x68
        if (addr == DS3231_ADDR)
        {
            devices = 1;
            devAddr = DS3231_ADDR;
            break; // nie ma sensu skanować dalej
        }
    }

    return devices;
}



/*-------------------------- external I2C Timer---------------------*/
bool Timer::RTC_GetHms(int& hour, int& minute, int& second)
{
    uint8_t sec_bcd = 0, min_bcd = 0, hour_bcd = 0;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // ustaw pointer na 0x00
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);

    // restart + odczyt
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &sec_bcd,  I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &min_bcd,  I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &hour_bcd, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (err != ESP_OK) {
        return 1;
    }

    second = bcd2dec(sec_bcd);
    minute = bcd2dec(min_bcd);
    hour   = bcd2dec(hour_bcd & 0x3F);

    return ESP_OK;

}

bool Timer::RTC_SetHms(int hour, int minute, int second)
{
    TimeSec = Timer::hmsToSec(hour,minute,second); 

    uint8_t sec_bcd  = dec2bcd(second);
    uint8_t min_bcd  = dec2bcd(minute);
    uint8_t hour_bcd = dec2bcd(hour);
    hour_bcd &= 0x3F;

    uint8_t data[3] = { sec_bcd, min_bcd, hour_bcd };

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write(cmd, data, 3, true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (err != ESP_OK) {
        return 1;
    }
    return ESP_OK;
}

bool Timer::RTC_Ping()
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DS3231_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return err == ESP_OK;
}


/*--------------------------Timer---------------------*/


using namespace std::chrono_literals;


/* Uptime from ECU start till now in sec */ 
int Timer::getSysTime_sec() 
{

    using namespace std::chrono;
    
    uint64_t us = esp_timer_get_time();

    /* conversion to seconds */
    uint64_t sec = us / 1'000'000ULL;
    
#ifdef DEBUG
    constexpr uint64_t TIME_SCALE = 1000ULL;   // przyspieszenie czasu
#else
    constexpr uint64_t TIME_SCALE = 1ULL;
#endif

    sec = sec * TIME_SCALE;
    
    return sec;
}


void Timer::updateRtcTime()
{

    const int currentSysTimer_sec = Timer::getSysTime_sec();
    int hour, minute, second;

        if (true==Timer::RTC_Ping())
        {
           DS3231Available = true;
           Timer::RTC_GetHms(hour, minute, second);
           TimeSec = Timer::hmsToSec(hour,minute,second);

           LastRtcSyncSystime_sec = currentSysTimer_sec;
         
           if (false == DS3231PrevAvailableStatus)
           {
               Uart::printf ("connection with RTC DS3231 established\n");
           }
       }
       else
       {
           
           DS3231Available = false;
           if (true == DS3231PrevAvailableStatus)
           {
               Uart::printf ("connection with RTC not established (or lost), internal systick will be used\n");
           }
       }

     DS3231PrevAvailableStatus = DS3231Available;
        
}

int Timer::RTC_getTime_sec()
{
    if (true==DS3231Available)
    {
        return TimeSec;
    }
    else
    {
        constexpr uint64_t DAY_SEC = 24ULL * 60ULL * 60ULL;
        return (TimeSec + Timer::getSysTime_sec() - LastRtcSyncSystime_sec)% DAY_SEC;
    }

}

void Timer::init()
{
    I2C::init();
    Timer::updateRtcTime();
    
}

/* internal systimer in msec */
uint64_t Timer::getSysTime_ms()
{
    return esp_timer_get_time()/ 1000;
}
#endif