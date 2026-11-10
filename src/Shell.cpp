extern "C" 
{
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    /* for systeminfo*/
    #include "esp_heap_caps.h"
    #include "esp_chip_info.h"
    #include "esp_app_desc.h"

    #include "esp_wifi.h"

    #include <cstring>
    #include <cstdio>
}
#include "common.hpp"
//#include "WifiManager.hpp"
///#include "NvsManager.hpp"
#include "HwInput.hpp"
#include "Timer.hpp"
#include "Uart.hpp"
#include "BtSerial.hpp"
#include "Shell.hpp"
#include "I2c.hpp"
#include "LightSensor.hpp"
#include "RhTempSensor.hpp"
#include "controller.hpp"
#include "Button.hpp"
#include "Logger.hpp"
#include "History.hpp"


namespace ShellCmd
{
   void printConfig();

   void handle(const char* cmd);
   void help(); 
   void version();
   void system();
   void i2cScan();
   void showHwio();
   void showHistory(); 

}

namespace
{
    
    constexpr int CMD_BUFFER_SIZE = 64;
    char CmdBuffer[CMD_BUFFER_SIZE];
    int CmdIndex = 0;

    /* shell messages history - 3 messages in histpory buffer */
    static char History[3][CMD_BUFFER_SIZE];
    static int  HistoryCount = 0;      // how much saved in buffer
    static int  HistoryIndex = -1;     // -1 = no active position
    static int  escState = 0;          // 0 = normal, 1 = ESC, 2 = ESC[
}

namespace ShellMsg
{
    constexpr const char* InvalidParam = "Invalid param\r\n";
    constexpr const char* UnknownCommand = "Unknown command. Type 'help' for available commands\r\n";

}

namespace ShellIO
{
    char read();
    void write(char c);
    void print(const char* str);
    void printf(const char* format, ...);
    void println(const char* str);
    bool available();
}


void Shell::init( )
{

    ShellIO::println(" ");
    ShellIO::println("PPFD light meter for plants - Shell");
    ShellIO::print("> ");

}

void Shell::Periodic()
{
    while(ShellIO::available())
    {
        char c = ShellIO::read();

        /* --- PARSER of ESC sequence [ X (Arrows) --- */

        if (escState == 0 && c == 0x1B)   /* ESC */
        {
            escState = 1;
            continue;
        }

        if (escState == 1 && c == '[')    /* ESC [ */
        {
            escState = 2;
            continue;
        }

        if (escState == 2)
        {
            if (c == 'A')   // UP
            {
                if (HistoryCount > 0)
                {
                    if (HistoryIndex < HistoryCount - 1)
                        HistoryIndex++;
                }

                /* clear actual line */
                while (CmdIndex > 0)
                {
                    ShellIO::write('\b');
                    ShellIO::write(' ');
                    ShellIO::write('\b');
                    CmdIndex--;
                }

                if (HistoryIndex >= 0)
                {
                    strncpy(CmdBuffer, History[HistoryIndex], CMD_BUFFER_SIZE);
                    CmdIndex = strlen(CmdBuffer);
                    ShellIO::print(CmdBuffer);
                }
            }
            else if (c == 'B') // DOWN
            {
                if (HistoryIndex > 0)
                {
                    HistoryIndex--;
                }
                else
                {
                    HistoryIndex = -1;
                }

                // clear line
                while (CmdIndex > 0)
                {
                    ShellIO::write('\b');
                    ShellIO::write(' ');
                    ShellIO::write('\b');
                    CmdIndex--;
                }

                if (HistoryIndex >= 0)
                {
                    strncpy(CmdBuffer, History[HistoryIndex], CMD_BUFFER_SIZE);
                    CmdIndex = strlen(CmdBuffer);
                    ShellIO::print(CmdBuffer);
                }
                else
                {
                    CmdBuffer[0] = '\0';
                    CmdIndex = 0;
                }
            }

            escState = 0;
            continue;
        }

        /* --- BACKSPACE --- */
        if (c == 0x08 || c == 0x7F)
        {
            if (CmdIndex > 0)
            {
                CmdIndex--;
                CmdBuffer[CmdIndex] = '\0';

                ShellIO::write('\b');
                ShellIO::write(' ');
                ShellIO::write('\b');
            }
            continue;
        }

        /* --- Normal digits --- */
        ShellIO::write(c);

        if(c == '\r' || c == '\n')
        {
            CmdBuffer[CmdIndex] = '\0';
            ShellIO::print("\r\n");

            ShellCmd::handle(CmdBuffer);

            CmdIndex = 0;
            CmdBuffer[0] = '\0';
        }
        else
        {
            if(CmdIndex < (CMD_BUFFER_SIZE - 1))
            {
                CmdBuffer[CmdIndex++] = c;
            }
        }
    }
}



void ShellCmd::handle(const char* cmd)
{
int temp{};
char profile[16];
int argc = 0;

    if (strcmp(cmd, "help") == 0)
    {
        ShellCmd::help();
    }
    else if(strcmp(cmd, "version") == 0)
    {
        ShellCmd::version();
    }
    else if(strcmp(cmd, "show") == 0)
    {
        ShellCmd::printConfig();
    }
    else if(strcmp(cmd, "showhwio") == 0)
    {
        ShellCmd::showHwio();
    }
    else if(strcmp(cmd, "history") == 0)
    {
        ShellCmd::showHistory();
    }
    else if(strcmp(cmd, "clearhistory") == 0)
    {
        History::clear();
    }
    else if(strcmp(cmd, "sysinfo") == 0)
    {
        ShellCmd::system();
    }    
    else if(strcmp(cmd, "i2cscan") == 0)
    {
        ShellCmd::i2cScan();
    }
    else if (strcmp(cmd, "nvsclear") == 0)
    {
        ShellIO::println("clearing Nvs and restarting...");
      //  NvsManager::clearNvs();
              ShellIO::println("TODO");

    }
    else if (strcmp(cmd, "restart") == 0)
    {
        ShellIO::println("restarting...");
        esp_restart();
    }
    else if (strcmp(cmd, "blestop") == 0)
    {
        ShellIO::println("Stop Bluetooth...");
        BtSerial::disable();
    }
    else if(sscanf(cmd,"setscreensaver %d",&temp) == 1)
    {
        controller::setScreensaverEnabled(temp);    
    }
    else if (strncmp(cmd, "setprofile", 10) == 0)
    {
        if (sscanf(cmd, "setprofile %15s", profile) != 1)       
        {
            ShellIO::println("Available params:");
            ShellIO::println("led3200 (warm led), led4000, led6500 (cold led), sun");
        }
        else if (strcmp(profile, "led3200") == 0)
        {
            LightSensor::setProfile(Lux2PpfdProfile::Led3200K );
            ShellIO::println("Current light measurement profile: LED 3200K");
        }
        else if (strcmp(profile, "led4000") == 0)
        {
            LightSensor::setProfile(Lux2PpfdProfile::Led4000K );
            ShellIO::println("Current light measurement profile: LED 4000K");

        }
        else if (strcmp(profile, "led6500") == 0)
        {
            LightSensor::setProfile(Lux2PpfdProfile::Led6500K );
            ShellIO::println("Current light measurement profile: LED 6500K");

        }
        else if (strcmp(profile, "sun") == 0)
        {
            LightSensor::setProfile(Lux2PpfdProfile::Sun );
            ShellIO::println("Current light measurement profile: sun");

        }
        else
        {
            ShellIO::println(ShellMsg::InvalidParam);
            ShellIO::println("Available params:");
            ShellIO::println("led3200 (warm led), led4000, led6500 (cold led), sun");
        }
 
    }
    else if (strncmp(cmd, "startlog", 8) == 0)
    {
        int hh{0};
        int mm{0};
        int sampleOffset_min{0};
        int recordingTime_h{0};

        argc = sscanf(cmd, "startlog %d:%d %d %d",&hh,&mm, &sampleOffset_min,&recordingTime_h);

            if ( argc == 4)    
            {
                if ((hh<24 && hh>=0) && (mm<60 && mm>=0) &&
                    (sampleOffset_min > 0) && (recordingTime > 0))
                {
                    Timer::SetTimeHms(hh, mm, 0);
                    Logger::start(hh,mm, sampleOffset_min*60,recordingTime_h);
                    ShellIO::println(" Logger started"); 
                }
                else ShellIO::println(ShellMsg::InvalidArgument);
            
            }
             else if (argc == 0)
            {
                Timer::SetTimeHms(0, 0, 0);
                Logger::start(0,0, 5*60,24);
                ShellIO::println(" Logger started on default parameters... "); 
            } 
            else
            {      
                ShellIO::println(ShellMsg::InvalidArgument); 
                ShellIO::println("startlog HH:MM offset recTime"); 
                ShellIO::println(" HH:MM - set first sample in log "); 
                ShellIO::println(" offset - time interval between samples (min) "); 
                ShellIO::println(" recTime - time of recording (h) "); 
            }
    }
    else if (strncmp(cmd, "stoplog", 7) == 0)
    {
        ShellIO::println("Data recording stopped."); 
        Logger::stop();
    }
    else
    {
       ShellIO::println(ShellMsg::UnknownCommand);  
    }

    // na końcu, przed promptem:
    if (cmd[0] != '\0')
    {
        // przesuwamy historię w dół
        for (int i = 2; i > 0; --i)
        {
            strncpy(History[i], History[i-1], CMD_BUFFER_SIZE);
        }
        strncpy(History[0], cmd, CMD_BUFFER_SIZE);

        if (HistoryCount < 3)
            HistoryCount++;

        HistoryIndex = -1; // reset pozycji historii po wykonaniu komendy
    }

    ShellIO::print("> ");
}

void ShellCmd::help()
{
    ShellIO::println("Available commands:");
    ShellIO::println(" ");
    ShellIO::print(" help\r\n");
    ShellIO::print(" version\r\n");
    ShellIO::print(" show\r\n"); 
    ShellIO::print(" showhwio\r\n"); 
    ShellIO::print(" sysinfo\r\n");
    ShellIO::print(" i2cscan\r\n"); 
    ShellIO::print(" nvsclear\r\n");  
    ShellIO::print(" restart\r\n");  
    ShellIO::print(" blestop\r\n");  
    ShellIO::print(" setprofile\r\n");  
    ShellIO::print(" history\r\n");  
    ShellIO::print(" clearhistory\r\n");  
    ShellIO::print(" startlog\r\n");
    ShellIO::print(" stoplog\r\n");  
}

void ShellCmd::version()
{
    const char* BUILD_DATE = __DATE__;
    const char* BUILD_TIME = __TIME__;

    ShellIO::printf("Build           : %s %s\r\n", BUILD_DATE, BUILD_TIME);

    const esp_app_desc_t* app = esp_app_get_description();
    ShellIO::printf("Project         : %s\r\n", app->project_name);
    ShellIO::printf("Git Version     : %s\r\n", app->version);
}


void ShellCmd::printConfig()
{
    int lux = LightSensor::getLux();
    int ppfd = LightSensor::getPpfd();

    #ifdef DEBUG
        ShellIO::println("DEBUG MODE\r\n");
    #endif
    ShellIO::println(" ");

    ShellIO::printf("Lux  : %d \n\r",lux);
    ShellIO::printf("PPFD : %d \n\r",ppfd);
    float Temp = RhTempSensor::getTemp();
    float Rh = RhTempSensor::getRh();
    ShellIO::printf ("Temperature  :    %f\n\r",Temp);
    ShellIO::printf ("RH Sensor    :    %f\n\r",Rh);
    ShellIO::println (" ");
    ShellIO::printf ("Ble state       :    %s\n\r",BtSerial::isBleActive()?"ON":"OFF");
    switch (Logger::getLoggerState() )
    {
        case loggerState::Waiting:     ShellIO::println ("Logger state    :    Idle"); break;
        case loggerState::Recording:   ShellIO::println ("Logger state    :    Recording"); break;
        case loggerState::Error:       ShellIO::println ("Logger state    :    Error"); break;
        case loggerState::Done:        ShellIO::println ("Logger state    :    Done"); break;
    default:
        break;
    }

}

void ShellCmd:: system()
{
    #if 0
    ShellIO::printf(
    "Shell stack free: %u\n",
    uxTaskGetStackHighWaterMark(shellTaskHandle));
    #endif

  //  int msec =  esp_timer_get_time()/ 1000;

    int msec = Timer::getSysTime_ms();
    ShellIO::printf("Systime            : %d msec\r\n", msec); 
    ShellIO::println("");

    ShellIO::printf("Free heap          : %u\r\n",  heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    ShellIO::printf("Min heap           : %u\r\n",  heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));

    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    ShellIO::printf("Cores              : %d\r\n",  chipInfo.cores);
    ShellIO::printf("Revision           : %d\r\n",  chipInfo.revision);
    ShellIO::println("");

    ShellIO::printf( "WiFi              : %s\r\n", (chipInfo.features & CHIP_FEATURE_WIFI_BGN) ? "YES" : "NO");
    ShellIO::printf("BLE                : %s\r\n",  (chipInfo.features & CHIP_FEATURE_BLE) ? "YES" : "NO");
    uint8_t mac[6];

    ShellIO::printf("BLE     status: %s \r\n", BtSerial::isBleActive() ? "Active" : "Disabled");

    esp_wifi_get_mac(WIFI_IF_STA, mac);

    ShellIO::printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void ShellCmd::i2cScan()
{

    int devAddr[I2C::devicesNmb] = {};
    /*to do - correct I2C::scan API */
    int devCount = I2C::scan(&devAddr[0]);

    for(int i = 0;i<devCount;i++)
    {
  
        ShellIO::printf("Device %s found at 0x%02X\r\n", I2C::GetDeviceName( devAddr[i]), devAddr[i]);
    }

     ShellIO::printf("Found %d known I2C device(s) \r\n", devCount );
}

void ShellCmd::showHwio()
{

    int lightSensorRaw = LightSensor::readRaw();

    ShellIO::printf ("light Sensor Raw:    %d\n\r",lightSensorRaw);

    uint16_t rawTemp = RhTempSensor::getTemp();
    uint16_t rawRh = RhTempSensor::getRh();
    ShellIO::printf ("Temperature Raw :    %d\n\r",rawTemp);
    ShellIO::printf ("RH Sensor Raw   :    %d\n\r",rawRh);
    ShellIO::println ("");
    ShellIO::printf ("Button          :    %d\n\r",Button::isOn());


}

void ShellCmd::showHistory()
{
    int index = History::getBufferHead();
    int time_min = 0;
    for(int i=0;i<History::BufferSize; i++ )
    {
        const MeasurementData* data =  History::getSample(index);
        if (data == nullptr) return;
        time_min = (i-History::BufferSize) * 5; //todo:: history logging offset 
        ShellIO::printf("%4d[min] lux:%d ppfd:%d t:%f rh:%f\n\r", time_min,  data->lux, data->ppfd, data->temperature, data->humidity);
        index = (index + 1 ) %History::BufferSize;
        
    }
}

/*-------------- Shell IO -------------------*/
char ShellIO::read()
{
    if (BtSerial::isBleActive())
       return BtSerial::read();
    else
        return Uart::read();
}

void ShellIO::write(char c)
{
    if (BtSerial::isBleActive())
        BtSerial::write(c);
    else
        Uart::write(c);
}

void ShellIO::print(const char* str)
{
    if (BtSerial::isBleActive())
        BtSerial::print(str);
    else
        Uart::print(str);
}

void ShellIO::println(const char* str)
{
    if (BtSerial::isBleActive())
       BtSerial::println(str);
    else
        Uart::println(str);
}

void ShellIO::printf(const char* format, ...)
{
    char buf[256];

    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    if (BtSerial::isBleActive())
        BtSerial::printf(buf);
    else
        Uart::printf(buf);
}


bool ShellIO::available()
{
     return BtSerial::isBleActive() ? BtSerial::available() : Uart::available();
    //return Uart::available();
}
