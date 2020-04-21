#include "wifitransfer.h"
#include "configmanager.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <sstream>
#include <list>
#include <time.h>
#include <cJSON.h>
#include "FS.h"
#include "SD_MMC.h"

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

        typedef enum {
            STATUS_NOTSTARTED = 0, 
            STATUS_INPROGRESS = 1,
            STATUS_COMPLETED = 2,
            STATUS_CANCELLED = 3,
            STATUS_TERMINATED = 4} session_status;

        WiFiStateMachine()   
        {
            reset();
        }


        wifi_state getState() {
            return _state;
        }

        bool upload_file(File &file_stream, const String& date_string="2020-04-17 09:50:22")
        {
            if (file_stream.available() > 0 && _httpClient.begin(_wifiClient, create_url_with_token("/api/device/upload").c_str()))
            {
                //Replace if existing
                _httpClient.addHeader("Content-Type", "application/octet-stream", true, true);
                _httpClient.addHeader("X-Id-Session", String(_id_session).c_str());
                _httpClient.addHeader("X-Filename", file_stream.name());
                //TODO set date...
                _httpClient.addHeader("X-Filedate", date_string);

                //POST
                //int httpCode = _httpClient.POST("Hello World!");
                //Send data from stream
                printf("Sending file: %s size: %i \n", file_stream.name(), file_stream.available());
                int httpCode = _httpClient.sendRequest("POST",&file_stream, file_stream.available());
                printf("Return code : %i\n", httpCode);
                
                _httpClient.end(); 


                return httpCode == HTTP_CODE_OK;
            }

           

            return false;
        }

        bool create_session(const String &session_name="MyName", const String &date_string="2020-04-17 09:50:22-0400", size_t duration = 0)
        {
            cJSON *root=cJSON_CreateObject();
            cJSON *session = cJSON_CreateObject();
            cJSON_AddItemToObject(root, "session", session);

            //Session info
            cJSON_AddNumberToObject(session, "id_session", 0);
            cJSON_AddNumberToObject(session, "id_session_type", _id_session_type);
            cJSON_AddStringToObject(session, "session_name", session_name.c_str());
            cJSON_AddStringToObject(session, "session_start_datetime", date_string.c_str());
            cJSON_AddNumberToObject(session, "session_status", STATUS_COMPLETED);
            cJSON_AddNumberToObject(session, "session_duration", duration);

            //Session participants
            cJSON *participants_array =  cJSON_AddArrayToObject(session, "session_participants");
            for (auto iter = _participants.begin(); iter != _participants.end(); iter++)
            {
                auto string_item = cJSON_CreateString((*iter).c_str());
                cJSON_AddItemToArray(participants_array, string_item);
            }

            std::string result(cJSON_Print(root));
            printf("Create session result: \n%s\n", result.c_str());

            //Free memory
            cJSON_free(root);

            //POST session information           
            if (_httpClient.begin(_wifiClient, create_url_with_token("/api/device/sessions").c_str()))
            {
                _httpClient.addHeader("Content-Type", "application/json");

                //POST
                int httpCode = _httpClient.POST(result.c_str()); 

                if (httpCode == HTTP_CODE_OK) {
                    String payload = _httpClient.getString();
                    printf("post result %s\n", payload.c_str());
                    //Parse JSON
                    cJSON *root = cJSON_Parse(payload.c_str());

                    if (cJSON_HasObjectItem(root, "id_session")) {
                        _id_session = cJSON_GetObjectItem(root,"id_session")->valueint;
                    }

                    //Free memory
                    cJSON_free(root);
                }

                _httpClient.end();
            }
            return (_id_session != -1);
        }

        void reset()
        {
            _config =  ConfigManager::instance()->getOpenTeraConfig();
            _state = STATE_DISCONNECTED;
            _id_device = -1;
            _id_project = -1;
            _id_session_type = -1;
            _id_session = -1;
            _participants.clear();

        }

        bool login()
        {
            if (_httpClient.begin(_wifiClient, create_url_with_token("/api/device/login").c_str()))
            {
                //GET
                int httpCode = _httpClient.GET();                
                //printf("login() - GET code: %i\n", httpCode);

                if (httpCode == HTTP_CODE_OK) {
                    String payload = _httpClient.getString();
                    //printf("data %s\n", payload.c_str());
                    //Parse JSON
                    cJSON *root = cJSON_Parse(payload.c_str());

                    //Handle device info
                    if (cJSON_HasObjectItem(root, "device_info"))
                    {                        
                        auto device_info_item = cJSON_GetObjectItem(root, "device_info");
                        //printf("device_info_item : %p \n", device_info_item);                        
                        if (cJSON_HasObjectItem(device_info_item, "id_device"))
                        {                            
                            _id_device = cJSON_GetObjectItem(device_info_item,"id_device")->valueint;
                            //printf("id_device: %i\n", _id_device);
                        }                        
                    }

                    if (cJSON_HasObjectItem(root, "participants_info"))
                    {
                        //printf("found participants_info\n");
                        auto participants_array = cJSON_GetObjectItem(root, "participants_info");
                        //printf("array size: %i\n",cJSON_GetArraySize(participants_array));
                        //TODO handle multiple participants
                        for (auto i = 0; i < cJSON_GetArraySize(participants_array); i++)
                        {
                            auto participant_item = cJSON_GetArrayItem(participants_array, i);
                            _id_project = cJSON_GetObjectItem(participant_item,"id_project")->valueint;
                            _participants.push_back(cJSON_GetObjectItem(participant_item,"participant_uuid")->valuestring);
                            //printf("id_project: %i, id_participant: %i \n", _id_project, _id_participant);
                        }
                    }

                    if (cJSON_HasObjectItem(root, "session_types_info"))
                    {
                        //printf("found session_types_info \n");
                        auto session_types_array = cJSON_GetObjectItem(root, "session_types_info");
                        //printf("array size: %i\n",cJSON_GetArraySize(session_types_array));
                        //TODO handle multiple session_types
                        for (auto i = 0; i < cJSON_GetArraySize(session_types_array); i++)
                        {
                            auto session_type_item = cJSON_GetArrayItem(session_types_array, i);
                            _id_session_type = cJSON_GetObjectItem(session_type_item,"id_session_type")->valueint;
                            //printf("id_session_type: %i\n", _id_session_type);
                        }
                    }
                    //Free memory
                    cJSON_free(root);
                }
                _httpClient.end();
            }

            printf("login result: %i %i %i %i \n", _id_device, _participants.size(), _id_project, _id_session_type);

            // Returns true if all information is valid
            return (_id_device != -1 && !_participants.empty() && _id_project != -1 && _id_session_type != -1);
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
        std::list<std::string> _participants;
        int _id_device;
        int _id_project;
        int _id_session_type;
        int _id_session;

    };

    String getTimeStringFromTimestamp(const time_t &timestamp)
    {
        struct tm *timeInfo = localtime(&timestamp);
        char timeStringBuff[50]; //50 chars should be enough
        strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", timeInfo);
        return String(timeStringBuff);
    }

    void wifiTransferTask(void *pvParameters)
    {
        printf("wifiTransferTask starting...\n");
        WiFiTransfer* wifi = reinterpret_cast<WiFiTransfer*>(pvParameters);
        assert(wifi);

        WiFiStateMachine machine;
        

        //TODO WiFi Stuff...
        wifi->initialize_wifi();

        delay(10000);
        
        machine.reset();            

        if (!machine.login())
        {
            printf("Cannot login\n");
            wifi->stop_transfer();
        }

        //Read files from devices
        if(SD_MMC.begin()){
            printf("SdCard opened type: %i \n", SD_MMC.cardType());

            File root = SD_MMC.open("/");

            if (root)
            {
                File file = root.openNextFile();
                while(file){
                    //Look for log directory
                    if(file.isDirectory()) {

                        if (String(file.name()).startsWith("/log")) {

                            // TODO Extract start date and time from log directory

                            printf("Found directory: %s\n", file.name());
                            ///log_20200415_133021 --> 2020-04-15 13:30:21-0400
                            // date = (date.substring(5,8) + "-" + date.substring(9,10) + "-" + date.substring(11,12));

                            time_t start_timestamp = file.getLastWrite();
                            String start_date = getTimeStringFromTimestamp(start_timestamp);
                            printf("******* Testing start date: %s \n", start_date.c_str());

                            File dir_root = SD_MMC.open(file.name());

                            while(dir_root) {
                                
                                printf("Found file: %s\n", dir_root.name());
                                String filename(dir_root.name());

                                if (filename.endsWith(".mdat"))
                                {
                                    time_t end_timestamp = dir_root.getLastWrite();
                                    String end_date = getTimeStringFromTimestamp(end_timestamp);
                                    printf("******* Testing end date: %s \n", end_date.c_str());
                                    time_t duration = end_timestamp - start_timestamp;
                                    if (!machine.create_session(file.name(), start_date, duration))
                                    {
                                        printf("Cannot create session\n");
                                        break;
                                    }

                                    //Upload file
                                    if (!machine.upload_file(dir_root, end_date))
                                    {
                                        printf("Cannot upload file\n");
                                        break;
                                    }

                                    //TODO upload log?
                                    //TODO Terminate session...

                                }
                                dir_root = dir_root.openNextFile();
                            }
                        }
                    }

                    //Find next file / directory
                    file = root.openNextFile();                         
                }
            }
    
        }   

        //Shutdown wifi module
        wifi->terminate_wifi();

        //Will terminate task
        wifi->stop_transfer();
    }
}//anonymous namespace

WiFiTransfer* WiFiTransfer::instance()
{
    if (WiFiTransfer::_instance == NULL)
        WiFiTransfer::_instance = new WiFiTransfer();
    return WiFiTransfer::_instance;
}



WiFiTransfer::WiFiTransfer()
{
    _mutex = xSemaphoreCreateMutex();
    
}

void WiFiTransfer::lock()
{
    assert(xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE);
}

void WiFiTransfer::unlock()
{
    assert(xSemaphoreGive(_mutex) == pdTRUE);
}

void WiFiTransfer::start_transfer()
{
    //initialize_wifi();
    //xTaskCreate(&wifiTransferTask, "WiFiTransferTask", 8192, this, 1, &_wifiTransferTaskHandle);
    //Same as Arduino core, IDLE task priority
    lock();
    xTaskCreatePinnedToCore(&wifiTransferTask, "WiFiTransferTask", 8192, this, 0, &_wifiTransferTaskHandle, 1);
    unlock();
}

void WiFiTransfer::stop_transfer()
{
    lock();
     if (_wifiTransferTaskHandle)
        vTaskDelete(_wifiTransferTaskHandle); 
    _wifiTransferTaskHandle = NULL;
    unlock();
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

void WiFiTransfer::terminate_wifi()
{
    //WiFi 0ff = trye, EraseApp = true
    WiFi.disconnect(true, true);
}
