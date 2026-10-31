#include <cstring>

#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "common.hpp"
#include "WifiManager.hpp"
/// E8:3D:C1:9E:1B:98
///
namespace
{


#ifdef SENDER
    constexpr size_t kExpectedFrameSize = sizeof(WifiManager::responseFrame);
    WifiManager::responseFrame gLastFrame{};
    WifiManager::ControlFrame gLastTrcvFrame{};
    constexpr uint8_t kPeerMac[6] =
    {
        0xE8, 0x3D, 0xC1, 0x9E, 0x1B, 0x98
    };
#else
    constexpr size_t kExpectedFrameSize = sizeof(WifiManager::ControlFrame);
    WifiManager::ControlFrame gLastFrame{};
    WifiManager::responseFrame gLastTrcvFrame{};

    constexpr uint8_t kPeerMac[6] =
    {
        0x44,0xB1,0x76,0x04,0xEF,0x18
    };
#endif
    volatile bool gFrameAvailable = false;

    bool sInitialized = false;
    bool sLastTxOk = false;
    uint32_t lastRxTick = xTaskGetTickCount();

    uint16_t sendedPackets = -1;
    uint16_t receivedPackets = 0;
   // void onDataSent(const wifi_tx_info_t* info, esp_now_send_status_t status);
   // void onDataRecv(const esp_now_recv_info_t* info, const uint8_t* data,int len);
    
}

void onDataSent(const wifi_tx_info_t* info,
                esp_now_send_status_t status)
{
    (void)info;

    sLastTxOk = (status == ESP_NOW_SEND_SUCCESS);
    /* todo zastanowić sie czy liczyć wszystkie wyslane czy tylko te z sukcesem*/
    sendedPackets++;
    
   // if (sLastTxOk)
     //   printf("frame send!!!");

}

void onDataRecv(const esp_now_recv_info_t* info, const uint8_t* data,  int len)
{
    (void)info;

    if (len != kExpectedFrameSize)
    {
        return;
    }

    memcpy(&gLastFrame, data, kExpectedFrameSize);
    lastRxTick = xTaskGetTickCount();
    gFrameAvailable = true;
    receivedPackets ++;
    /* first response frame, connection established , start calculating sended and received packets */
    if (sendedPackets==-1)
    {
        sendedPackets = 1;
    }

}

int WifiManager::init()
{
    ESP_ERROR_CHECK(esp_netif_init());

    esp_err_t err = esp_event_loop_create_default();
    if ((err != ESP_OK) && (err != ESP_ERR_INVALID_STATE))
    {
        return -1;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    uint8_t mac[6];

    ESP_ERROR_CHECK(
        esp_wifi_get_mac(WIFI_IF_STA, mac));

    printf("ESP-NOW MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           mac[0],
           mac[1],
           mac[2],
           mac[3],
           mac[4],
           mac[5]);

    if (esp_now_init() != ESP_OK)
    {
        return -2;
    }

    ESP_ERROR_CHECK(esp_now_register_send_cb(onDataSent));

    ESP_ERROR_CHECK(esp_now_register_recv_cb(onDataRecv));

//#ifdef SENDER

    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, kPeerMac, 6);

    peer.channel = 0;
    peer.encrypt = false;

    if (esp_now_add_peer(&peer) != ESP_OK)
    {
        return -3;
    }

//#endif

    sInitialized = true;

    uint8_t primary;
    wifi_second_chan_t second;

    esp_wifi_get_channel(&primary, &second);

    printf("Wifi Channel: %u\n", primary);

    return 0;
}

#ifdef SENDER
int WifiManager::send(const ControlFrame& frame)
{
    if (!sInitialized)
    {
        return -1;
    }

    esp_err_t result =
        esp_now_send( kPeerMac, reinterpret_cast<const uint8_t*>(&frame), sizeof(frame));

    if (result != ESP_OK)
    {
        return -2;
    }
    else
    {
        gLastTrcvFrame = frame;
    }

    return 0;
}

bool WifiManager::receiveResponse(responseFrame& frame)
{
    if (!gFrameAvailable)
    {
        return false;
    }

    frame = gLastFrame;
    gFrameAvailable = false;

    return true;
}
#else
int WifiManager::send(const responseFrame& frame)
{
    if (!sInitialized)
    {
        return -1;
    }

    esp_err_t result =
        esp_now_send(
            kPeerMac,
            reinterpret_cast<const uint8_t*>(&frame),
            sizeof(frame));

    if (result != ESP_OK)
    {
        return -2;
    }
    else
    {
        gLastTrcvFrame = frame;
    }
    return 0;
}

bool WifiManager::receiveResponse(ControlFrame& frame)
{
    if (!gFrameAvailable)
    {
        return false;
    }

    frame = gLastFrame;
    gFrameAvailable = false;

    return true;
}
#endif


void WifiManager::getLastFrame(ControlFrame& frame,responseFrame& frame_resp,bool& txStatus)
{
    #ifdef SENDER
    frame_resp = gLastFrame;
    frame = gLastTrcvFrame;
    #else
    frame = gLastFrame;
    frame_resp = gLastTrcvFrame;
    #endif
    txStatus = sLastTxOk;

}

bool WifiManager::isConnected()
{
    return (xTaskGetTickCount() - lastRxTick)
           < pdMS_TO_TICKS(500);

}

int WifiManager::getLostPacketsNum()
{
    return sendedPackets - receivedPackets;
}
