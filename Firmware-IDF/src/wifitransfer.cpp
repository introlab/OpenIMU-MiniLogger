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

WiFiTransfer* WiFiTransfer::_instance = NULL;


#define EXAMPLE_ESP_MAXIMUM_RETRY  10

namespace 
{
    static EventGroupHandle_t s_wifi_event_group;
    static int s_retry_num = 0;

    static const char *TAG = "WiFiTransfer";
    static const char *EXAMPLE_ESP_WIFI_SSID = "test";
    static const char *EXAMPLE_ESP_WIFI_PASS = "test";

    /* The event group allows multiple bits for each event, but we only care about one event 
    * - are we connected to the AP with an IP? */
    const int WIFI_CONNECTED_BIT = BIT0;

    static void event_handler2(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) 
    {
        

    }

    static esp_err_t event_handler(void *ctx, system_event_t *event)
    {   
        switch(event->event_id) {
            case SYSTEM_EVENT_STA_START:
                esp_wifi_connect();
            break;

            case SYSTEM_EVENT_STA_GOT_IP:
                ESP_LOGI(TAG, "got ip:%s",ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
                s_retry_num = 0;
                xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                break;


            case SYSTEM_EVENT_STA_DISCONNECTED:
            {
                if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
                    esp_wifi_connect();
                    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                    s_retry_num++;
                    ESP_LOGI(TAG,"retry to connect to the AP");
                }
                ESP_LOGI(TAG,"connect to the AP fail\n");
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
  
    //xTaskCreate(&wifiTransferTask, "WiFiTransferTask", 2048, this, 6, &_wifiTransferTaskHandle);
    initialize_wifi();
}

void WiFiTransfer::initialize_wifi()
{
    printf("WiFiTransfer::initialize_wifi() \n");

    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, this) );


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    //TODO, THIS IS A PATCH TO AVOID BROWNOUT WHEN ENABLING WIFI
    //uint32_t brown_reg_temp = READ_PERI_REG(RTC_CNTL_BROWN_OUT_REG); //save WatchDog register
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    //sleep(1);
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, brown_reg_temp); //put back brownout detector register

    //FOR IDF 4.0, we are using 3.3 right now
    //ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    //ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, SYSTEM_EVENT_STA_GOT_IP, &event_handler, NULL) );
    
    wifi_config_t wifi_config;

    strcpy((char*) wifi_config.sta.ssid, EXAMPLE_ESP_WIFI_SSID);
    strcpy((char*) wifi_config.sta.password, EXAMPLE_ESP_WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             wifi_config.sta.ssid, wifi_config.sta.password);

    //ESP_LOGI(TAG, "esp_wifi_set_ps().");
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
}