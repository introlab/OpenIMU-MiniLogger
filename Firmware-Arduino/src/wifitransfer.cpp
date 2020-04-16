#include "wifitransfer.h"
#include "configmanager.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <sstream>

WiFiTransfer* WiFiTransfer::_instance = NULL;

namespace 
{

    WiFiClientSecure g_wifiClient;

    void wifiTransferTask(void *pvParameters)
    {
        printf("wifiTransferTask starting...\n");
        WiFiTransfer* wifi = reinterpret_cast<WiFiTransfer*>(pvParameters);
        assert(wifi);

        //Initialize last tick
        TickType_t _lastTick = xTaskGetTickCount();

        OpenTeraConfig_Sd  teraConfig = ConfigManager::instance()->getOpenTeraConfig();

        std::ostringstream url;
        
        url << "https://" << teraConfig.openTeraServerName 
            << ":" << teraConfig.openTeraServerPort 
            << "/api/device/login?token=" << teraConfig.openTeraToken;

        

        //TODO WiFi Stuff...
        while(1)
        {
            delay(10000);
            printf("Connnecting to server %s\n", url.str().c_str());

            //Secure client
       
            HTTPClient https;
            if (https.begin(g_wifiClient, url.str().c_str())) {  

                //GET
                int httpCode = https.GET();
                
                printf("GET code: %i\n", httpCode);

                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                    String payload = https.getString();
                    printf("data %s\n", payload.c_str());
                }

                https.end();
        
            }



        } //while(1)

        

        
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
    initialize_wifi();
    xTaskCreate(&wifiTransferTask, "WiFiTransferTask", 8192, this, 6, &_wifiTransferTaskHandle);
}

void WiFiTransfer::initialize_wifi()
{

    WiFiConfig_Sd wifiConfig = ConfigManager::instance()->getWiFiConfig();
    printf("Connecting to wifi: %s, %s\n",wifiConfig.ssid.c_str(),  wifiConfig.password.c_str());

    
    WiFi.begin(wifiConfig.ssid.c_str(), wifiConfig.password.c_str());

    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        // wait 1 second for re-trying
        delay(1000);
    }

    WiFi.printDiag(Serial);
}