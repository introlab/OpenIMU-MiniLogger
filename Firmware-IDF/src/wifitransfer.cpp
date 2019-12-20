#include "wifitransfer.h"

#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "configmanager.h"



WiFiTransfer* WiFiTransfer::_instance = NULL;


#define EXAMPLE_ESP_MAXIMUM_RETRY  10

namespace 
{
    static EventGroupHandle_t s_wifi_event_group;


    static const char *TAG = "WiFiTransfer";


    /* The event group allows multiple bits for each event, but we only care about one event 
    * - are we connected to the AP with an IP? */
    const int WIFI_CONNECTED_BIT = BIT0;

    static esp_err_t event_handler(void *ctx, system_event_t *event)
    {   
        switch(event->event_id) {
            case SYSTEM_EVENT_STA_START:
                ESP_LOGI(TAG,"SYSTEM_EVENT_STA_START...\n");
                esp_wifi_connect();
            break;

            case SYSTEM_EVENT_STA_GOT_IP:
                ESP_LOGI(TAG, "got ip:%s",ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
                xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                break;


            case SYSTEM_EVENT_STA_DISCONNECTED:
            {
                ESP_LOGI(TAG,"SYSTEM_EVENT_STA_DISCONNECTED...\n");
                //Try to connect
                esp_wifi_connect();
                xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                break;
            }

            default:
                break;    
        }


        return ESP_OK;
    }

    void wifiTransferTask(void *pvParameters)
    {
        printf("powerTask starting...\n");
        WiFiTransfer* wifi = reinterpret_cast<WiFiTransfer*>(pvParameters);
        assert(wifi);

        //Initialize last tick
        TickType_t _lastTick = xTaskGetTickCount();

        //TODO WiFi Stuff...


        

        
    }
}


WiFiTransfer* WiFiTransfer::instance()
{
    if (WiFiTransfer::_instance == NULL)
        WiFiTransfer::_instance = new WiFiTransfer();
    return WiFiTransfer::_instance;
}



WiFiTransfer::WiFiTransfer()
{
    _mutex = xSemaphoreCreateMutex();
    xTaskCreate(&wifiTransferTask, "WiFiTransferTask", 2048, this, 6, &_wifiTransferTaskHandle);
    initialize_wifi();
}

void WiFiTransfer::initialize_wifi()
{
    printf("WiFiTransfer::initialize_wifi() \n");
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, this) );


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    
    wifi_config_t wifi_config;

    //Required to initialize the structure properly
    memset(&wifi_config, 0, sizeof(wifi_config_t));

    strcpy((char*) wifi_config.sta.ssid, ConfigManager::instance()->getWiFiConfig().ssid.c_str());
    strcpy((char*) wifi_config.sta.password, ConfigManager::instance()->getWiFiConfig().password.c_str());

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    //printf("before wifi start\n");
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             wifi_config.sta.ssid, wifi_config.sta.password);

    //ESP_LOGI(TAG, "esp_wifi_set_ps().");
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
}