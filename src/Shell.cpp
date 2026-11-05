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
#include "WifiManager.hpp"
///#include "NvsManager.hpp"
#include "HwInput.hpp"
//#include "Timer.hpp"
#include "Uart.hpp"
#include "BtSerial.hpp"
#include "Shell.hpp"
#include "I2c.hpp"
#include "LightSensor.hpp"
#include "controller.hpp"

namespace ShellCmd
{
   void printConfig();

   void handle(const char* cmd);
   void help(); 
   void version();
   void system();
   void i2cScan();
   void showHwio();

   void getLastFrame();

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
    ShellIO::println("PPFD light plant meter Shell");
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
   else if(sscanf(cmd,"setscreensaver %d",&temp) == 1)
    {
        controller::setScreensaverEnabled(temp);    
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
    ShellIO::print(" getlastframe\r\n");  

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

    #if 0
    if (true==hwSwitchInput::btEnblSwitchisON())
        ShellIO::println("BLE switch state  : ON");
    else
        ShellIO::println("BLE switch state  : OFF");
    #endif

    #ifdef DEBUG
        ShellIO::println("DEBUG MODE\r\n");
    #endif
    ShellIO::println(" ");

    bool connected = WifiManager::isConnected();

    ShellIO::printf("Lux  : %d \n\r",lux);
    ShellIO::printf("PPFD : %d /n/r",ppfd);



}

void ShellCmd:: system()
{
    #if 0
    ShellIO::printf(
    "Shell stack free: %u\n",
    uxTaskGetStackHighWaterMark(shellTaskHandle));

    ShellIO::printf(
    "Lamp stack free: %u\n",
    uxTaskGetStackHighWaterMark(lampTaskHandle));

    ShellIO::printf(
    "RTC stack free: %u\n",
    uxTaskGetStackHighWaterMark(rtcTaskHandle));
    #endif

  //  int msec =  esp_timer_get_time()/ 1000;

    //msec = Timer::getSysTime_ms();
  //  ShellIO::printf("Systime            : %d msec\r\n", msec); 

    ShellIO::printf("Free heap          : %u\r\n",  heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    ShellIO::printf("Min heap           : %u\r\n",  heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));

    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    ShellIO::printf("Cores              : %d\r\n",  chipInfo.cores);
    ShellIO::printf("Revision           : %d\r\n",  chipInfo.revision);

    ShellIO::printf( "WiFi              : %s\r\n", (chipInfo.features & CHIP_FEATURE_WIFI_BGN) ? "YES" : "NO");
    ShellIO::printf("BLE                : %s\r\n",  (chipInfo.features & CHIP_FEATURE_BLE) ? "YES" : "NO");
    uint8_t mac[6];

    ShellIO::printf("BLE     status: %s \r\n", BtSerial::isBleActive() ? "Active" : "Disabled");

    esp_wifi_get_mac(WIFI_IF_STA, mac);

    ShellIO::printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
    mac[0], mac[1], mac[2],
    mac[3], mac[4], mac[5]);
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

    ShellIO::printf ("lightSensorRaw:    %d\n\r",lightSensorRaw);

}

void ShellCmd::getLastFrame()
{




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