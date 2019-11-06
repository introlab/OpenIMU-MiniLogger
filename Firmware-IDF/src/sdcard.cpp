#include "sdcard.h"
#include "ioexpander.h"
#include <stdio.h>
#include <cJSON.h>

//Static instance
SDCard* SDCard::_instance = NULL;

//Anonymous namespace to avoid name collision with other modules
namespace sdcard
{

    void generateTimestamp(void *pvParameters)
    {
        SDCard *sdcard = reinterpret_cast<SDCard*>(pvParameters);
        assert(sdcard);

        TickType_t lastGeneration = xTaskGetTickCount();
        timestampSendable_t now;
        time(&now.data);
        // struct tm *ts;
        while(1) {
            vTaskDelayUntil(&lastGeneration, 1000 / portTICK_RATE_MS);
            time(&now.data);
            // ts = localtime(&now);
            // Serial.printf("%s", asctime(ts));
            sdcard->enqueue(now,false);
            
                //Serial.println("giving data ready");
                // xSemaphoreGive(_dataReadySemaphore);
            
        }
    }


    /**
     * Interrupt handler 
     */
    // void IRAM_ATTR sdcard_gpio_isr_handler(void* arg)
    // {
    //     SDCard *sdcard = reinterpret_cast<SDCard*>(arg);
    //     assert(sdcard);

    //     //TODO Handle timestamp drifts and update...
    //     //get time
    //     timestampSendable_t now;
    //     time(&now.data);
        
    //     //Post time to queue, from isr
    //     sdcard->enqueue(now, true);
    // }

    void logTask(void *pvParameters)
    {
        SDCard *sdcard = reinterpret_cast<SDCard*>(pvParameters);
        assert(sdcard);

        printf("SDCard LogTask starting... \n");

        timestampSendable_t timestamp;
        int imu_cnt = 0;
        int gps_cnt = 0;
        int power_cnt = 0;
        int baro_cnt = 0;
        int pulse_cnt = 0;

        while(1)
        {
            //Waiting for new data from any source
            xSemaphoreTake(sdcard->getDataReadySemaphore(), portMAX_DELAY);

            // Timestamp received
            if(xQueueReceive(sdcard->getTimestampQueue(), &timestamp.data, 0) == pdTRUE) 
            {

                //_logFile.write('t');
                //_logFile.write(timestamp.bytes, sizeof(time_t));
                sdcard->logFileWrite("t", 1);
                sdcard->logFileWrite(timestamp.bytes, sizeof(time_t));

                //Should sync file
                sdcard->syncFile();

                printf("Timestamp %li i: %i g: %i p: %i b: %i h: %i\n", timestamp.data, 
                    imu_cnt, gps_cnt, power_cnt, baro_cnt, pulse_cnt);

                //Reset counters
                imu_cnt = 0;
                gps_cnt = 0;
                power_cnt = 0;
                baro_cnt = 0;
                pulse_cnt = 0;

            }

            // Data from IMU
            imuDataPtr_t imuPtr = nullptr;
            if(xQueueReceive(sdcard->getIMUQueue(), &imuPtr, 0) == pdTRUE) 
            {
                //_logFile.write('i');
                //_logFile.write((uint8_t*) imuPtr, sizeof(imuData_t));
                sdcard->logFileWrite("i", 1);
                sdcard->logFileWrite(imuPtr, sizeof(imuData_t));
                free(imuPtr);
                imu_cnt++;
            }

            //Data from Power
            powerDataPtr_t powerPtr = nullptr;
            if (xQueueReceive(sdcard->getPowerQueue(), &powerPtr, 0) == pdTRUE)
            {
                //_logFile.write('p');
                //_logFile.write((uint8_t*) powerPtr, sizeof(powerData_t));
                sdcard->logFileWrite("p",1);
                sdcard->logFileWrite(powerPtr, sizeof(powerData_t));
                free(powerPtr);
                power_cnt++;
            }

            // Data from Barometer
            baroDataPtr_t baroPtr;
            if(xQueueReceive(sdcard->getBaroQueue(), &baroPtr, 0) == pdTRUE)
            {
                //_logFile.write('b');
                //_logFile.write((uint8_t*) baroPtr, sizeof(baroData_t));
                // printf("saving baro : %f %f \n", baroPtr->pressure, baroPtr->temperature);
                sdcard->logFileWrite("b", 1);
                sdcard->logFileWrite(baroPtr, sizeof(baroData_t));
                free(baroPtr);
                baro_cnt++;
            }
            
            // Data from GPS
            gpsDataPtr_t gpsPtr;
            if(xQueueReceive(sdcard->getGPSQueue(), &gpsPtr, 0) == pdTRUE)
            {
                //_logFile.write('g');
                //_logFile.write((uint8_t*)gpsPtr, sizeof(gpsData_t));
                sdcard->logFileWrite("g",1);
                sdcard->logFileWrite(gpsPtr, sizeof(gpsData_t));
                free(gpsPtr);
                gps_cnt++;
            }

            pulseDataPtr_t pulsePtr = nullptr;
            if(xQueueReceive(sdcard->getPulseQueue(), &pulsePtr, 0) == pdTRUE)
            {
                //_logFile.write('h');
                //_logFile.write((uint8_t*)pulsePtr, sizeof(pulseData_t));
                sdcard->logFileWrite("h",1);
                sdcard->logFileWrite(pulsePtr, sizeof(pulseData_t));
                free(pulsePtr);
                pulse_cnt++;
            }
        }

    }
}

void checkSDtask(void *pvParameters)
{
    
    TickType_t _lastTick = xTaskGetTickCount();

    while(1)
    {
        vTaskDelayUntil(&_lastTick, 300 / portTICK_RATE_MS);

        SDCard::instance()->checkSD();    
    }

}

SDCard* SDCard::instance()
{
    if (SDCard::_instance == NULL)
        SDCard::_instance = new SDCard();
    return SDCard::_instance;
}


SDCard::SDCard()
    :   _logTaskHandle(NULL), 
        _imuQueue(NULL),  
        _gpsQueue(NULL), 
        _powerQueue(NULL), 
        _baroQueue(NULL),
        _timestampQueue(NULL),
        _pulseQueue(NULL),
        _dataReadySemaphore(NULL), 
        _logFile(NULL), 
        _mutex(NULL)
{
    //Init mutex
    _mutex = xSemaphoreCreateMutex();
    assert(_mutex != NULL);

    //TODO, unused for now
    IOExpander::instance().pinMode(EXT_PIN04_SD_N_CD, INPUT);
    IOExpander::instance().pullupMode(EXT_PIN04_SD_N_CD, HIGH);

    //PIN EXT_PIN05_SD_SEL (select 1=ESP32, 0 = USB2640)
    //EXT_PIN03_SD_N_ENABLED (output enable 0=NO SD Card)
    IOExpander::instance().pinMode(EXT_PIN03_SD_N_ENABLED, OUTPUT);
    IOExpander::instance().pinMode(EXT_PIN05_SD_SEL, OUTPUT);

    //SD-MMC all pins as inputs before initializing module
    setup_gpio(0);
    setup_gpio(2);
    setup_gpio(4);
    setup_gpio(12);
    setup_gpio(13);

    _card = NULL;

    //Setup host
    _host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    _slot_config = SDMMC_SLOT_CONFIG_DEFAULT();    

    toESP32();
    //toExternal();

    xTaskCreate(&checkSDtask, "IsSDPresent", 2048, this, 10, nullptr);
}

SDCard::~SDCard()
{

}

void SDCard::setup_gpio(int pin)
{
    gpio_config_t io_conf;
    //interrupt on falling edge
    io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO33
    io_conf.pin_bit_mask =  (1ULL << pin);
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t) 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

}

void SDCard::setup_interrupt_pin(bool enable)
{
    gpio_config_t io_conf;
    //interrupt on falling edge if enabled
    if (enable)
        io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_NEGEDGE;
    else
        io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;

    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO33
    io_conf.pin_bit_mask =  (1ULL << PIN_INTERRUPT_FROM_GPS_REF);
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t) 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


    //Set ISR if enabled
    // if (enable)
    //     gpio_isr_handler_add((gpio_num_t)PIN_INTERRUPT_FROM_GPS_REF, sdcard::sdcard_gpio_isr_handler, this);
    // else
    //     gpio_isr_handler_remove((gpio_num_t)PIN_INTERRUPT_FROM_GPS_REF);
}

int SDCard::toESP32()
{
    printf("SD to ESP32 \n");
    unmount();

    //Select ESP32 for SD
    IOExpander::instance().digitalWrite(EXT_PIN05_SD_SEL,HIGH);

    //Output enable (inverted)
    IOExpander::instance().digitalWrite(EXT_PIN03_SD_N_ENABLED,HIGH);
    vTaskDelay(500 / portTICK_RATE_MS);
    IOExpander::instance().digitalWrite(EXT_PIN03_SD_N_ENABLED,LOW);
    vTaskDelay(500 / portTICK_RATE_MS);

    // Mount SD Card
    if(mount()) {
        printf("Mounted successfull\n");
        return 1;
    //    listDir(SD_MMC, "/", 0);
    }
    return -1;
}

void SDCard::toExternal()
{
    printf("SD to external\n");

    unmount();

    //Select USB for SD
    IOExpander::instance().digitalWrite(EXT_PIN05_SD_SEL,LOW);

    //Output enable (inverted)
    IOExpander::instance().digitalWrite(EXT_PIN03_SD_N_ENABLED,HIGH);
    vTaskDelay(500 / portTICK_RATE_MS);
    IOExpander::instance().digitalWrite(EXT_PIN03_SD_N_ENABLED,LOW);
    vTaskDelay(500 / portTICK_RATE_MS);
}


bool SDCard::mount()
{
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config;
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;

    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &_host, &_slot_config, &mount_config, &_card);
    if (ret == ESP_OK)
    {
        //Print card info
        sdmmc_card_print_info(stdout, _card);
        return true;
    }

    printf("Error mounting file system\n");
    return false;
}

void SDCard::unmount()
{
    if (_logFile)
        fclose(_logFile);

    _logFile = NULL;

    esp_vfs_fat_sdmmc_unmount();
}


void SDCard::startLog()
{
    //Make sure we are connected to the SD card!
    //printf("Beginning logging\n");
    toESP32();
    //Look for latest file
    //Create it if not 
    struct stat st;
    int file_id = 0;

    if(SdCardPresent)
    {

        if (stat("/sdcard/latest.txt", &st) != 0)
        {
            printf("latest file not found.Creating...\n");
            //Write first index to the file
            FILE* f = fopen("/sdcard/latest.txt", "w");
            printf("After trying to open file\n");
            vTaskDelay(2000);
            fprintf(f,"%i\n",0);
            vTaskDelay(2000);
            printf("After fprintf()\n");
            fclose(f);
            printf("After fclose\n");
        }
        else
        {
            //Read latest file
            FILE *f = fopen("/sdcard/latest.txt", "r");
            fscanf(f,"%i", &file_id);
            //Increment file id
            file_id++;
            fclose(f);
            printf("file ID is now : %i\n", file_id);

            //Write back file id to file
            f = fopen("/sdcard/latest.txt", "w");
            fprintf(f,"%i\n",file_id);
            fclose(f);

        }

        //Safety, if we were already logging
        if (_logFile)
            fclose(_logFile);

        //Open binary file
        char logfile_name[32];
        sprintf(logfile_name, "/sdcard/%i.dat", file_id);
        printf("Log file name %s \n", logfile_name);
        _logFile = fopen(logfile_name,"w");
        assert(_logFile);
        
        //Write header...
        logFileWrite("h",1);

        //Create queues
        lock();
        _dataReadySemaphore = xSemaphoreCreateCounting(128, 0);
        _timestampQueue = xQueueCreate(20, sizeof(time_t));
        _imuQueue = xQueueCreate(20, sizeof(imuData_t*));
        _powerQueue = xQueueCreate(20, sizeof(powerData_t*));
        _baroQueue = xQueueCreate(20, sizeof(baroData_t*));
        _gpsQueue = xQueueCreate(20, sizeof(gpsData_t*));
        _pulseQueue = xQueueCreate(20, sizeof(pulseData_t*));
        unlock();

        //Create task
        xTaskCreate(&sdcard::logTask, "LogTask", 4096, this, 10, &_logTaskHandle);
        xTaskCreatePinnedToCore(&sdcard::generateTimestamp, "SD card log", 2048, this, 15, &_timestampTask, 1);

        //Enable interrupt
        setup_interrupt_pin(true);
    }
    else
    {
        printf("No Sd Card found\n");
    }
}

void SDCard::stopLog()
{
    //Disable interrupt
    setup_interrupt_pin(false);

    //Destroy task
    if (_logTaskHandle)
        vTaskDelete(_logTaskHandle); 
    _logTaskHandle = NULL;

    if (_timestampTask)
        vTaskDelete(_timestampTask); 
    _timestampTask = NULL;

    //Close file
    syncFile();
    if (_logFile)
    {
        fclose(_logFile);
        _logFile = NULL;
    }

    //Destroy queues
    lock();
    if (_timestampQueue)
        vQueueDelete(_timestampQueue); 
    _timestampQueue = NULL;

    if (_imuQueue)
        vQueueDelete(_imuQueue); 
    _imuQueue = NULL;

    if (_powerQueue)
        vQueueDelete(_powerQueue);
    _powerQueue = NULL;

    if (_baroQueue)
        vQueueDelete(_baroQueue); 
    _baroQueue = NULL;

    if (_gpsQueue)
        vQueueDelete(_gpsQueue); 
    _gpsQueue = NULL;

     if (_pulseQueue)
        vQueueDelete(_pulseQueue); 
    _pulseQueue = NULL;

    //Destroy semaphore
    if (_dataReadySemaphore)
        vSemaphoreDelete(_dataReadySemaphore); 
    _dataReadySemaphore = NULL;

    unlock();
}


bool SDCard::enqueue(imuDataPtr_t data, bool from_isr)
{
    lock(from_isr);
    if (data != nullptr && _imuQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_imuQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                unlock(from_isr);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_imuQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                unlock(from_isr);
                return true;
            }
        }
    }
    unlock(from_isr);
    return false;
}

bool SDCard::enqueue(timestampSendable_t data, bool from_isr)
{
    /**
     *  THIS IS CALLED FROM AN ISR. BE CAREFUL. 
     *  NO NEED TO PROTECT QUEUES, WE KNOW THEY ARE VALID
     */
    lock(from_isr);
    if (_timestampQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_timestampQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                unlock(from_isr);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_timestampQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                unlock(from_isr);
                return true;
            }
        }
    }
    return false;
}

//Data from Power
bool SDCard::enqueue(powerDataPtr_t data, bool from_isr)
{
    lock(from_isr);
    if (data != nullptr && _powerQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_powerQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                unlock(from_isr);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_powerQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                unlock(from_isr);
                return true;
            }
        }
    }
    unlock(from_isr);
    return false;
}


bool SDCard::enqueue(baroDataPtr_t data, bool from_isr)
{
    lock(from_isr);
    if (data != nullptr && _baroQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_baroQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                unlock(from_isr);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_baroQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                unlock(from_isr);
                return true;
            }
        }
    }
    unlock(from_isr);
    return false;
}

bool SDCard::enqueue(pulseDataPtr_t data, bool from_isr)
{
    lock(from_isr);
    if (data != nullptr && _pulseQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_pulseQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                unlock(from_isr);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_pulseQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                unlock(from_isr);
                return true;
            }
        }
    }
    unlock(from_isr);
    return false;
}

bool SDCard::enqueue(gpsDataPtr_t data, bool from_isr)
{
    lock(from_isr);
    if (data != nullptr && _gpsQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_gpsQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                unlock(from_isr);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_gpsQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                unlock(from_isr);
                return true;
            }
        }
    }
    unlock(from_isr);
    return false;
}

bool SDCard::logFileWrite(const void* data, size_t size)
{
    if (_logFile)
    {
        if (fwrite(data, size, 1, _logFile) == 1)
            return true;
    }

    printf("Error writing to log file\n");
    return false;
}

bool SDCard::syncFile()
{
    if (_logFile)
    {
        fflush(_logFile);
        fsync(fileno(_logFile));
        return true;
    }
    printf("Error syncing log file\n");
    return false;
}

void SDCard::lock(bool from_isr)
{
    if (from_isr)
        xSemaphoreTakeFromISR(_mutex, NULL);
    else
        assert(xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE);
}

void SDCard::unlock(bool from_isr)
{   
    if (from_isr)
        xSemaphoreGiveFromISR(_mutex, NULL);
    else
        assert(xSemaphoreGive(_mutex) == pdTRUE);
}

bool SDCard::GetOpenTeraConfigFromSd(OpenTeraConfig_Sd *OpenTeraSdConfig)
{
    toESP32();

    return false;
}

bool SDCard::GetIMUConfigFromSd(IMUconfig_Sd *IMUSdConfig)
{
    toESP32();

    struct stat stt;

    //if (stat("/sdcard/PARAME~1/STARTI~1.JSO", &stt) != 0)
    if (stat("/sdcard/ParameterFolder/StartingParameter.json", &stt) != 0)
    {
        printf("No folder found\n");
        return false;
    }
    else   
    {
        
        //The ESP-32 sees the name differently from what it is when we create the file using the computer
        //FILE* f = fopen("/sdcard/PARAME~1/STARTI~1.JSO","r");
        FILE* f = fopen("/sdcard/ParameterFolder/StartingParameter.json","r");

        if (f==NULL)
        {
            printf("Can't open the directory\n");
            return false;
        }

        //Go to end of file
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        printf("JSON file size returns : %li\n", size);

        //Go to begin of file
        fseek(f, 0 , SEEK_SET);
       
        //Allocate string with null character at the end 
        char* json_string = (char*) malloc(size + 1);
        memset(json_string,0, size +1);

        //Read the complete file
        fread(json_string, 1, size, f);
        printf("json : %s \n", json_string);

        //Parse JSON
        cJSON *root = cJSON_Parse(json_string);

        //Structure is initialized with default values. Replace value if found in JSON file
        if (cJSON_HasObjectItem(root, "samplerate"))
            IMUSdConfig->IMUSampleRate = cJSON_GetObjectItem(root,"samplerate")->valueint;

        if (cJSON_HasObjectItem(root, "setupaccel"))
            IMUSdConfig->IMUAcellRange = cJSON_GetObjectItem(root,"setupaccel")->valueint;

        if (cJSON_HasObjectItem(root, "setupgyro"))
            IMUSdConfig->IMUGyroRange = cJSON_GetObjectItem(root,"setupgyro")->valueint;

        //Free memory
        cJSON_free(root);
        free(json_string);

        //Close file
        fclose(f);

    }

    return true;
}

void SDCard::checkSD()
{
    if (IOExpander::instance().digitalRead(EXT_PIN04_SD_N_CD)==0)
    {
        SdCardPresent=true;
    }
    else if (IOExpander::instance().digitalRead(EXT_PIN04_SD_N_CD)==1)
    {
        SdCardPresent=false;
    }
}

bool SDCard::getSdCardPresent()
{
    return SdCardPresent;
}