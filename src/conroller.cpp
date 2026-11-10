#include "common.hpp"

#include "HwInput.hpp"
#include "WifiManager.hpp"
#include "controller.hpp"
#include "OledDisplay.hpp"
#include "LightSensor.hpp"
#include "RhTempSensor.hpp"
#include "Button.hpp"
#include "Timer.hpp"
#include "BtSerial.hpp"
#include "Logger.hpp"
#include "HistoryLog.hpp"

bool ScreenSaverEnabled = false;
uint64_t screensaverTimer_sec = 0; 

void controller::init()
{
    screensaverTimer_sec = Timer::getTime_sec();
    ScreenSaverEnabled = false;

    HistoryLog::init();
}


void controller::periodic()
{
    MeasurementData data{};   
    data.lux = SENSOR_ERROR;
    data.ppfd = SENSOR_ERROR;
    data.humidity = SENSOR_ERROR;
    data.temperature = SENSOR_ERROR;

    ConfigData config;
    config.Profile = LightSensor::getProfile();
    config.loggingInterval_s = 5*60;
    memcpy (config.fileName,"log0000.csv",sizeof("log0000.csv"));

    StatusData  status;
    status.BleStatus = BtSerial::isBleActive();
    status.recording = Logger::getLoggerState();

    Button::update();
    LightSensor::update();
    RhTempSensor::update();
    BtSerial::Periodic();


    data.lux = LightSensor::getLux();
    data.ppfd = LightSensor::getPpfd();
    data.temperature = static_cast<uint32_t> (RhTempSensor::getTemp());
    data.humidity = static_cast<uint32_t> (RhTempSensor::getRh());



    if( (Timer::getTime_sec() - screensaverTimer_sec) > 5*60)
    {
        ScreenSaverEnabled = true;
        if( (Timer::getTime_sec() - screensaverTimer_sec) > 5*60)
        {
            //todo disable oled
        }

    }

    switch(Button::getEvent())
    {
        case Button::Event::OneClick:
            screensaverTimer_sec = Timer::getTime_sec();
            ScreenSaverEnabled = false;
            break;
        case Button::Event::DoubleClick:
            screensaverTimer_sec = Timer::getTime_sec();
            ScreenSaverEnabled = false;
            BtSerial::enable();

            break;    
        case Button::Event::LongPress:
            screensaverTimer_sec = Timer::getTime_sec();
            ScreenSaverEnabled = false;
            Logger::start(00,00,20,24);
            status.recording=Logger::getLoggerState();

            break;
        case Button::Event::Released:
            /* do nothing */
            break;
    }

    Logger::update( data);
    HistoryLog::update(data);

    OledDisplay::drawScreen(data,config,status);

    if (ScreenSaverEnabled)
    {
        OledDisplay::drawScreenSaver();
    }

    OledDisplay::Refresh();

}

void controller::setScreensaverEnabled(bool enabled)
{
    ScreenSaverEnabled = enabled;
}
