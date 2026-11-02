#include <stdio.h>

#include "driver/spi_common.h"
#include "driver/spi_master.h"

#include "esp_vfs_fat.h"

#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
/*
ESP32-C3      microSD

MOSI  ------> DI
MISO  <------ DO
SCK   ------> CLK
CS    ------> CS
3V3   ------> VCC
GND   ------> GND

*/

bool sdMounted = false;

void Logger::logData(int timestamp, int lux, int ppfd, int temp, int humidity)
{
  if (!sdMounted)
  {
      return;
  }
    FILE* f = fopen("/sdcard/log.csv", "a");
    
    if (f)
    {
        fprintf(f, "%lu;%u;%u;%.1f;%.1f\n", timestamp, lux, ppfd, temp, humidity);
    
        fclose(f);
    }
}

void Logger::startLogging()
{
  FILE *f = fopen("/sdcard/log.csv", "w");

  if(f)
  {
      fprintf(f,"time;lux;ppfd;temp;rh\n");
  
      fclose(f);
  }
}

#define PIN_NUM_MISO 4
#define PIN_NUM_MOSI 5
#define PIN_NUM_CLK  6
#define PIN_NUM_CS   7

void Logger::Init()
{
  InitSpi();
  SdCardInit();
}
void Logger::InitSpi()
{
  spi_bus_config_t bus_cfg = {
      .mosi_io_num = PIN_NUM_MOSI,
      .miso_io_num = PIN_NUM_MISO,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4000,
  };
  
  ESP_ERROR_CHECK(
      spi_bus_initialize(
          SDSPI_HOST_DEFAULT().slot,
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

slot_config.gpio_cs = PIN_NUM_CS;
slot_config.host_id = host.slot;

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

#If 0
Logger::UnMountSD()
{
  fflush(...)
  fclose(...)
 esp_vfs_fat_sdcard_unmount("/sdcard", card);
}

// common.h

struct MeasurementData
{
    uint32_t timestamp;

    uint32_t lux;
    uint32_t ppfd;

    float temperature;
    float humidity;

    float batteryVoltage;
    float dli;
};

/*
LightSensor      100-200 ms
Temp/Humidity    2-5 s
Display          500 ms
Logger           5 min


*/
#endif
