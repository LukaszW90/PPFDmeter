
#include <string.h>
extern "C" {
#include "esp_log.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "host/ble_hs.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
}

#include "Timer.hpp"
#include "HwInput.hpp"
#include "BtSerial.hpp"


constexpr uint64_t Timeout_sec = 120;
static const char *TAG = "BtSerial";

static uint8_t  rx_buf[256];
static int      rx_len = 0;

static uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t tx_handle   = 0;
static uint16_t rx_handle   = 0;

static const char *adv_name = "ESP32";

static const ble_uuid128_t UART_SERVICE_UUID =
    BLE_UUID128_INIT(
        0x9E,0xCA,0xDC,0x24,
        0x0E,0xE5,
        0xA9,0xE0,
        0x93,0xF3,
        0xA3,0xB5,
        0x01,0x00,0x40,0x6E);

static const ble_uuid128_t UART_RX_UUID =
    BLE_UUID128_INIT(
        0x9E,0xCA,0xDC,0x24,
        0x0E,0xE5,
        0xA9,0xE0,
        0x93,0xF3,
        0xA3,0xB5,
        0x02,0x00,0x40,0x6E);

static const ble_uuid128_t UART_TX_UUID =
    BLE_UUID128_INIT(
        0x9E,0xCA,0xDC,0x24,
        0x0E,0xE5,
        0xA9,0xE0,
        0x93,0xF3,
        0xA3,0xB5,
        0x03,0x00,0x40,0x6E);

bool sBteActive = false;


static int on_rx_write( uint16_t conn, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int on_tx_access( uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gap_event( struct ble_gap_event *event, void *arg);
static void start_advertising();
static void ble_on_sync(void);
static void host_task(void *param);
static void notify_data( const uint8_t *data, size_t len);
extern "C" void ble_on_reset(int reason);

static int on_rx_write(uint16_t conn, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    rx_len = ctxt->om->om_len;

    if (rx_len > sizeof(rx_buf))
        rx_len = sizeof(rx_buf);

    memcpy(rx_buf, ctxt->om->om_data, rx_len);
    ESP_LOGI(TAG, "RX write len=%d", rx_len);

    BtSerial::ResetTimeout();
    
    return 0;
}

static const struct ble_gatt_chr_def uart_chrs[] =
{
    {
        .uuid       = &UART_RX_UUID.u,
        .access_cb  = on_rx_write,
        .flags      = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
        .val_handle = &rx_handle,
    },
    {
        .uuid       = &UART_TX_UUID.u,
        .access_cb  = on_tx_access,
        .flags      = BLE_GATT_CHR_F_READ |
                      BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &tx_handle,
    },
    {0}
};

static const struct ble_gatt_svc_def gatt_svcs[] =
{
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &UART_SERVICE_UUID.u,
        .characteristics = uart_chrs,
    },
    {0}
};

uint64_t sLastActivityTime_sec = 0;


extern "C" void ble_on_reset(int reason)
{
    ESP_LOGE(TAG, "BLE reset reason=%d", reason);
}

static int on_tx_access( uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    return 0;
}


static void start_advertising()
{
    uint8_t addr_type;

    int rc = ble_hs_id_infer_auto(0, &addr_type);

    ESP_LOGI(TAG, "ble_hs_id_infer_auto rc=%d addr_type=%d", rc, addr_type);

    if (rc != 0)
        return;

    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    fields.name = (const uint8_t *)adv_name;
    fields.name_len = strlen(adv_name);
    fields.name_is_complete = 1;

    static const ble_uuid128_t adv_uuids[] =
    {
        BLE_UUID128_INIT(
            0x9E,0xCA,0xDC,0x24,
            0x0E,0xE5,
            0xA9,0xE0,
            0x93,0xF3,
            0xA3,0xB5,
            0x01,0x00,0x40,0x6E)
    };
    
    fields.uuids128 = adv_uuids;

    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);

    ESP_LOGI(TAG, "adv_set_fields rc=%d", rc);

    if (rc != 0)
        return;

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;

    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start( addr_type, NULL, BLE_HS_FOREVER, &adv_params, gap_event, NULL);

    ESP_LOGI(TAG, "adv_start rc=%d", rc);
}

static void ble_on_sync(void)
{
    ESP_LOGI(TAG, "BLE synced");

    start_advertising();
}

static void host_task(void *param)
{
    ESP_LOGI(TAG, "host_task started");

    nimble_port_run();

    nimble_port_freertos_deinit();
}

static int gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
        case BLE_GAP_EVENT_CONNECT:

            if (event->connect.status == 0)
            {
                conn_handle = event->connect.conn_handle;
                ESP_LOGI(TAG, "Connected handle=%d", conn_handle);
            }
            else
            {
                ESP_LOGW(TAG, "Connection failed status=%d", event->connect.status);
                start_advertising();
            }
            BtSerial::ResetTimeout();
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "Disconnected");

            conn_handle = BLE_HS_CONN_HANDLE_NONE;
            if (sBteActive)
                start_advertising();
            break;
        
        case BLE_GAP_EVENT_SUBSCRIBE:
                ESP_LOGI(TAG, "subscribe cur_notify=%d", event->subscribe.cur_notify);
            break;
        
        case BLE_GAP_EVENT_MTU:
                ESP_LOGI(TAG, "MTU updated=%d", event->mtu.value);
            break;
        
        default:
            break;
    }

    return 0;
}

void BtSerial::init(const char *name)
{
    ESP_LOGI(TAG, "init(%s)", name);

    adv_name = name;

    int rc = nimble_port_init();

    ESP_LOGI(TAG,"nimble_port_init rc=%d", rc);

    if (rc != 0)
        return;

    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.sync_cb  = ble_on_sync;

    ble_svc_gap_init();
    ble_svc_gatt_init();

   rc = ble_svc_gap_device_name_set(name);
   ESP_LOGI(TAG, "device_name_set rc=%d", rc);

   rc = ble_gatts_count_cfg(gatt_svcs);
   ESP_LOGI(TAG, "count_cfg rc=%d", rc);

   if(rc != 0)
   {
      ESP_LOGE(TAG, "count_cfg failed");
      return;
   }

   rc = ble_gatts_add_svcs(gatt_svcs);
   ESP_LOGI(TAG, "add_svcs rc=%d", rc);

  if(rc != 0)
  {
     ESP_LOGE(TAG, "add_svcs failed");
     return;
  }

  if (rc != 0)
  {
      ESP_LOGE(TAG, "add_svcs failed");
       return;
  }
   nimble_port_freertos_init(host_task);

   ResetTimeout();
   sBteActive = true;
}

void BtSerial::deinit()
{
    ESP_LOGI(TAG, "deinit");

    nimble_port_stop();
}

void BtSerial::begin(const char *name)
{
    init(name);
}

int BtSerial::available()
{
    return rx_len;
}


char BtSerial::read()
{
    if (rx_len == 0)
        return 0;

    char c = rx_buf[0];

    memmove(rx_buf, rx_buf + 1, --rx_len);

    return c;
}

static void notify_data( const uint8_t *data, size_t len)
{
    if (conn_handle == BLE_HS_CONN_HANDLE_NONE)
        return;

    if (tx_handle == 0)
        return;

    struct os_mbuf *om =  ble_hs_mbuf_from_flat( data,len);
    if (!om)
        return;

    int rc = ble_gatts_notify_custom( conn_handle, tx_handle, om);
    ESP_LOGI(TAG, "notify rc=%d", rc);
}

void BtSerial::write(char c)
{
    uint8_t b = (uint8_t)c;

    notify_data(&b, 1);
}

void BtSerial::print(const char *s)
{
    if (!s)
        return;
    notify_data( (const uint8_t *)s, strlen(s));
}


void BtSerial::printf(const char* format, ...)
{
    constexpr size_t BUFFER_SIZE = 256; 
    char buffer[BUFFER_SIZE];

    va_list args;
    va_start(args, format);

    int len = vsnprintf(buffer, BUFFER_SIZE, format, args);
    
    va_end(args);

    if (len > 0)
    {
        /* send in chunks if length greater than MTU */
        size_t sent = 0;
        while (sent < len)
        {
            size_t chunk = len - sent;
            if (chunk > 20) chunk = 20;  /* safe limit */
            
            notify_data((const uint8_t*)(buffer + sent), chunk);
            sent += chunk;
            vTaskDelay(pdMS_TO_TICKS(2));  /*small pause before next chunk */
        }
    }
}
void BtSerial::println(const char* str)
{
    BtSerial::printf(str);
    BtSerial::print("\n\r");
}
/* only for runtime enabling-disabling */
bool BtSerial::enable()
{
    if(true==sBteActive)
        return false; /* ble already enabled, return 0 */
    
    sBteActive = true;
    start_advertising();
    ESP_LOGD(TAG, "BLE enabled");
    
    ResetTimeout();

    return true; /* state enabled - return status 1 */

}

bool BtSerial::disable()
{
    if(false==sBteActive)
        return false; /* ble already disabled , return 0 - noting happened*/
    
    if(conn_handle != BLE_HS_CONN_HANDLE_NONE)
    {
        BtSerial::println("Timeout reached. BLE disabled.");
        ble_gap_terminate(conn_handle,  BLE_ERR_REM_USER_CONN_TERM);
    }

    int rc = ble_gap_adv_stop();
    ESP_LOGI(TAG, "adv_stop rc=%d", rc);

    sBteActive = false;
    ESP_LOGD(TAG, "BLE disabled");
    return true; /* state disabled - return status 1 */
}

/* Tiemout checking */
void BtSerial::Periodic()
{
    
    uint64_t currentTime_sec = Timer::getSysTime_sec();
    if (currentTime_sec - sLastActivityTime_sec > Timeout_sec)
    {
        disable();
    }
 
}

void BtSerial::ResetTimeout()
{
    sLastActivityTime_sec = Timer::getSysTime_sec();
}

bool BtSerial::isBleActive()
{
    return sBteActive;
}