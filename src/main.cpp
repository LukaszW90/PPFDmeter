#include <chrono>
#include <thread>

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    #include "esp_timer.h"
    #include "esp_log.h"

    #include <cmath>
}

#include "common.hpp"
#include "HwConfig.hpp"


//#include "Timer.hpp"
#include "common.hpp"
#include "HwInput.hpp"
#include "I2c.hpp"
#include "OledDisplay.hpp"
#include "Shell.hpp"
#include "Uart.hpp"
#include "BtSerial.hpp"
#include "WifiManager.hpp"
#include "NvsManager.hpp"
#include "controller.hpp"

namespace TaskPeriod_ms
{

    constexpr int Shell = 50;
    constexpr int Controller = 50;
}
///constexpr auto STEP_DELAY = std::chrono::milliseconds(STEP_DELAY_MS);



/* tasks */
void controller_task(void*);
void shell_task(void*);

/* all adventures started on void main... */

extern "C" void app_main() {

    hwSwitchInput::init();
    Uart::init();
    I2C::init();
    OledDisplay::init();
   /// Timer::init();
    NvsManager::initNvs(); 

   // BtSerial::init("GL_QS");
   // vTaskDelay(pdMS_TO_TICKS(1000));


    xTaskCreate(controller_task, "controller", 6144, nullptr, 1, nullptr);
    xTaskCreate(shell_task, "usb_shell", 4096, nullptr, 1, nullptr);
   
    vTaskSuspend(NULL);
}

void controller_task(void*)
{

 //   WifiManager::init();
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (true)
    {
        controller::periodic();

        vTaskDelay(pdMS_TO_TICKS(TaskPeriod_ms::Controller));
    }
}


void shell_task(void*)
{
    Shell::init();  

    while (true)
    {
     //   BtSerial::Periodic();
        Shell::Periodic();
        vTaskDelay(pdMS_TO_TICKS(TaskPeriod_ms::Shell));
    }
}