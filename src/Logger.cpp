#include "Logger.hpp"

#include <stdio.h>

#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"

#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "common.hpp"
#include "HwConfig.hpp"
#include "Timer.hpp"
#include "Logger.hpp"
/*
ESP32-C3      microSD

MOSI  ------> DI
MISO  <------ DO
SCK   ------> CLK
CS    ------> CS
3V3   ------> VCC
GND   ------> GND

*/

namespace
{
    loggerState CurrentState = loggerState::Waiting;
    bool sdMounted = false;
    uint32_t lastSaveTime_sec = 0;
    uint32_t startingTime_sec = 0;
    int sampleOffset_sec_ =  5*60;
    int recordingTime_h_ = 24;

   MeasurementData dataLog[32];
   int DataLogBufNmb = 0;
}

void Logger::update( MeasurementData& data)
{
    uint64_t currentTime = Timer::getSysTime_sec();
    int timestamp_min = Timer::getTime_sec();
    if(CurrentState==loggerState::Recording)
    {
        if(currentTime - lastSaveTime_sec > sampleOffset_sec_)
        {

            Logger::logData(timestamp_min, data);
            lastSaveTime_sec = Timer::getSysTime_sec();
        }

        if (currentTime - startingTime_sec >  180/*Timer::hmsToSec(recordingTime_h_,0) */)
        {
                CurrentState = loggerState::Done; //todo: state done
        }
    }

}

void OfflineLogUpadte( MeasurementData& data )
{
  int i = DataLogBufNmb%32;
  memcpy(&LogData[DataLogBufNmb],data, sizeof(MeasurementData));
    DataLogBufNmb++;
}


void Logger::logData(int timestamp, MeasurementData& data)
{
  if (!sdMounted)
  {
      return;
  }
    FILE* f = fopen("/sdcard/log0000.csv", "a");
    
    if (f)
    {
        fprintf(f, "%d; %lu;%lu; %0.1f; %0.1f\n", timestamp, data.lux, data.ppfd, data.temperature, data.humidity);
    
        fclose(f);
    }
}

void Logger::stop()
{
    CurrentState = loggerState::Waiting;
}
void Logger::start(const int hh, const int mm, uint64_t sampleOffset_sec, int recordingTime_h)
{
  FILE *f = fopen("/sdcard/log0000.csv", "w");

  if(f)
  {

    fprintf(f,"time;lux;ppfd;temp;rh\n");
  
    fclose(f);

    Timer::SetTimeHms(hh,mm,00);
    lastSaveTime_sec = Timer::getSysTime_sec();
    startingTime_sec = Timer::getSysTime_sec();
    sampleOffset_sec_ =  sampleOffset_sec;
    recordingTime_h_ = recordingTime_h;

    CurrentState = loggerState::Recording;
  }
  else
  {
    CurrentState = loggerState::Error;
  }
}

void Logger::init()
{
  InitSpi();
  SdCardInit();
}

void Logger::InitSpi()
{
  spi_bus_config_t bus_cfg = {
      .mosi_io_num = HwConfig::SpiMosiPin,
      .miso_io_num = HwConfig::SpiMisoPin,
      .sclk_io_num = HwConfig::SpiSckPin,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4000,
  };
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

  ESP_ERROR_CHECK(
      spi_bus_initialize(
          static_cast<spi_host_device_t>(host.slot),
          &bus_cfg,
          SDSPI_DEFAULT_DMA
      )
  );
}

void Logger::SdCardInit()
{
  /* mount sd card */
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();

sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();

slot_config.gpio_cs = HwConfig::SpiCsPin;
slot_config.host_id = static_cast<spi_host_device_t>(host.slot);

/* init filesystem */

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024
};

sdmmc_card_t* card;

esp_err_t ret = esp_vfs_fat_sdspi_mount( "/sdcard", &host, &slot_config, &mount_config, &card  );

  if(ret != ESP_OK)
  {
      sdMounted = false;
      printf("SD mount failed\n");
      return;
  }
  else
  {
    sdMounted = true;
  }
}

#if 0
Logger::UnMountSD()
{
  fflush(...)
  fclose(...)
 esp_vfs_fat_sdcard_unmount("/sdcard", card);
}
#endif


loggerState Logger::getLoggerState()
{
    return  CurrentState;
;
}
