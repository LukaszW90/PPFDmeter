extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    #include "nvs_flash.h"
    #include "nvs.h"
}

#include "NvsManager.hpp"
#include "Uart.hpp"

constexpr const char* NVS_NAMESPACE = "lamp";
constexpr const char* NVS_KEY_QUEENSLAND = "queensland";

NvsErrorCode NvsManager::initNvs()
{
    esp_err_t err = nvs_flash_init();
    NvsErrorCode retVal = NvsErrorCode::ERROR;
        
    if (
        err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND
    )
    {
        ESP_ERROR_CHECK(nvs_flash_erase());

        err = nvs_flash_init();
    }

    printf("nvs init\n");
    return NvsErrorCode::OK;
}


#if 0

NvsErrorCode NvsManager::save(const LampConfig_struct& config)
{
    nvs_handle_t handle;
    NvsErrorCode retVal = NvsErrorCode::ERROR;
    
    if (nvs_open( NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK)
    {
        esp_err_t err = nvs_set_blob( handle, NVS_KEY_QUEENSLAND, &config, sizeof(config));

        if (err == ESP_OK)
        {
            if (ESP_OK == nvs_commit(handle))
                retVal = NvsErrorCode::OK;
            
        }

        nvs_close(handle);
        
    }

    return retVal;
}


NvsErrorCode NvsManager::load(LampConfig_struct& config)
{
    nvs_handle_t handle;
    NvsErrorCode retVal = NvsErrorCode::ERROR;
    
    if (nvs_open( NVS_NAMESPACE, NVS_READONLY,&handle) == ESP_OK)
    {
        size_t size = sizeof(config);

        esp_err_t err = nvs_get_blob( handle, NVS_KEY_QUEENSLAND, &config, &size);

        nvs_close(handle);

        if (( ESP_OK == err) && (size == sizeof(config)))
        {
             retVal = NvsErrorCode::OK;
        }
    }
    
    return retVal;
}

NvsErrorCode NvsManager::clearNvs()
{
    if (nvs_flash_erase() == ESP_OK)
    {
        esp_restart();
        /* should not reach this point */
        return NvsErrorCode::OK;
    }

    return NvsErrorCode::ERROR;
}

#endif