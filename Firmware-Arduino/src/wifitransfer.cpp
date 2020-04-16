#include "wifitransfer.h"
#include "configmanager.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <sstream>
#include <cJSON.h>

WiFiTransfer* WiFiTransfer::_instance = NULL;

namespace 
{

    WiFiClientSecure g_wifiClient;

    class WiFiStateMachine
    {
        public:
        
        typedef enum {
            STATE_DISCONNECTED, 
            STATE_CONNECTED, 
            STATE_LOGIN } wifi_state;

        WiFiStateMachine()
            : _state(STATE_DISCONNECTED)
        {
            //Update configuration
            _config =  ConfigManager::instance()->getOpenTeraConfig();
            _id_device = -1;
            _id_project = -1;
            _id_participant = -1;
            _id_session_type = -1;
        }


        wifi_state getState() {
            return _state;
        }

        bool create_session()
        {
            return true;
        }


        bool login()
        {
            if (_httpClient.begin(_wifiClient, create_url_with_token("/api/device/login").c_str()))
            {
                //GET
                int httpCode = _httpClient.GET();                
                printf("login() - GET code: %i\n", httpCode);

                if (httpCode == HTTP_CODE_OK) {
                    String payload = _httpClient.getString();
                    printf("data %s\n", payload.c_str());
                    

                    //Parse JSON
                    cJSON *root = cJSON_Parse(payload.c_str());

                    //Handle device info
                    if (cJSON_HasObjectItem(root, "device_info"))
                    {
                        
                        auto device_info_item = cJSON_GetObjectItem(root, "device_info");
                        printf("device_info_item : %p \n", device_info_item);
                        
                        if (cJSON_HasObjectItem(device_info_item, "id_device"))
                        {
                            
                            _id_device = cJSON_GetObjectItem(device_info_item,"id_device")->valueint;
                            printf("id_device: %i\n", _id_device);
                        }
                        
                    }

                    if (cJSON_HasObjectItem(root, "participants_info"))
                    {
                        printf("found participants_info\n");
                        auto participants_array = cJSON_GetObjectItem(root, "participants_info");
                        printf("array size: %i\n",cJSON_GetArraySize(participants_array));
                        //TODO handle multiple participants
                        for (auto i = 0; i < cJSON_GetArraySize(participants_array); i++)
                        {
                            auto participant_item = cJSON_GetArrayItem(participants_array, i);
                            _id_project = cJSON_GetObjectItem(participant_item,"id_project")->valueint;
                            _id_participant = cJSON_GetObjectItem(participant_item,"id_participant")->valueint;
                            printf("id_project: %i, id_participant: %i \n", _id_project, _id_participant);
                        }

                    }

                    if (cJSON_HasObjectItem(root, "session_types_info"))
                    {
                        printf("found session_types_info \n");
                        auto session_types_array = cJSON_GetObjectItem(root, "session_types_info");
                        printf("array size: %i\n",cJSON_GetArraySize(session_types_array));
                        //TODO handle multiple session_types
                        for (auto i = 0; i < cJSON_GetArraySize(session_types_array); i++)
                        {
                            auto session_type_item = cJSON_GetArrayItem(session_types_array, i);
                            _id_session_type = cJSON_GetObjectItem(session_type_item,"id_session_type")->valueint;
                            printf("id_session_type: %i\n", _id_session_type);
                        }
                    }
                    //Free memory
                    cJSON_free(root);
                }
                _httpClient.end();
            }
            // Returns true if all information is valid
            return (_id_device != -1 && _id_participant != -1 && _id_project != -1 && _id_session_type != -1);
        }


        protected:

        std::string create_url_with_token(const std::string &endpoint)
        {
            std::ostringstream url;
            url << "https://" << _config.openTeraServerName 
            << ":" << _config.openTeraServerPort 
            << endpoint << "?token=" << _config.openTeraToken;
            return url.str();
        }

        wifi_state _state;
        OpenTeraConfig_Sd _config;
        HTTPClient _httpClient;
        WiFiClientSecure _wifiClient;
        int _id_device;
        int _id_project;
        int _id_participant;
        int _id_session_type;

    };

    void wifiTransferTask(void *pvParameters)
    {
        printf("wifiTransferTask starting...\n");
        WiFiTransfer* wifi = reinterpret_cast<WiFiTransfer*>(pvParameters);
        assert(wifi);

        WiFiStateMachine machine;
        

        //TODO WiFi Stuff...
        while(1)
        {
            delay(10000);
            
           

            if (!machine.login())
            {
                printf("Cannot login\n");
                continue;
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
    xTaskCreate(&wifiTransferTask, "WiFiTransferTask", 8192, this, 12, &_wifiTransferTaskHandle);
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