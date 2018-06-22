#include "sdcard.h"
#include "ioexpander.h"

//Static instance
SDCard* SDCard::_instance = NULL;

//Anonymous namespace to avoid name collision with other modules
namespace sdcard
{

    /**
     * Interrupt handler 
     */
    void IRAM_ATTR sdcard_gpio_isr_handler(void* arg)
    {
        SDCard *sdcard = reinterpret_cast<SDCard*>(arg);
        assert(sdcard);

        //TODO Handle timestamp drifts and update...
        //get time
 
        timestampSendable_t now;
        time(&now.data);


        //Post time to queue, from isr
        sdcard->enqueue(now, true);
    }

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

        while(1)
        {
            //Waiting for new data from any source
            xSemaphoreTake(sdcard->getDataReadySemaphore(), portMAX_DELAY);

            // Timestamp received
            if(xQueueReceive(sdcard->getTimestampQueue(), &timestamp.data, 0) == pdTRUE) 
            {
                printf("Timestamp %li i: %i g: %i p: %i b: %i\n", timestamp.data, 
                    imu_cnt, gps_cnt, power_cnt, baro_cnt);

                //Reset counters
                imu_cnt = 0;
                gps_cnt = 0;
                power_cnt = 0;
                baro_cnt = 0;

            }

            // Data from IMU
            imuDataPtr_t imuPtr = nullptr;
            if(xQueueReceive(sdcard->getIMUQueue(), &imuPtr, 0) == pdTRUE) 
            {
                //_logFile.write('i');
                //_logFile.write((uint8_t*) imuDataPtr, sizeof(imuData_t));
                
                //Free memory
                free(imuPtr);
                imu_cnt++;
            }

            //Data from Power
            powerDataPtr_t powerPtr = nullptr;
            if (xQueueReceive(sdcard->getPowerQueue(), &powerPtr, 0) == pdTRUE)
            {
                //_logFile.write('p');
                //_logFile.write((uint8_t*) powerDataPtr, sizeof(powerData_t));
                free(powerPtr);
                power_cnt++;
            }

            // Data from Barometer
            baroDataPtr_t baroPtr;
            if(xQueueReceive(sdcard->getBaroQueue(), &baroPtr, 0) == pdTRUE)
            {
                //_logFile.write('b');
                //_logFile.write((uint8_t*) baroDataPtr, sizeof(baroData_t));
                free(baroPtr);
                baro_cnt++;
            }
            

            
            
        }

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
        _dataReadySemaphore(NULL)
{

    _dataReadySemaphore = xSemaphoreCreateCounting(128, 0);
    _timestampQueue = xQueueCreate(20, sizeof(time_t));
    _imuQueue = xQueueCreate(20, sizeof(imuData_t*));
    _powerQueue = xQueueCreate(20, sizeof(powerData_t*));
    _baroQueue = xQueueCreate(20, sizeof(baroData_t*));

    //TODO, unused for now
    IOExpander::instance().pinMode(EXT_PIN04_SD_N_CD, INPUT);

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
    if (enable)
        gpio_isr_handler_add((gpio_num_t)PIN_INTERRUPT_FROM_GPS_REF, sdcard::sdcard_gpio_isr_handler, this);
    else
        gpio_isr_handler_remove((gpio_num_t)PIN_INTERRUPT_FROM_GPS_REF);
}

void SDCard::toESP32()
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
    //    listDir(SD_MMC, "/", 0);
    }
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
    //mount_config.allocation_unit_size = 16 * 1024;


    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &_host, &_slot_config, &mount_config, &_card);
    if (ret == ESP_OK)
    {
        //Print card info
        sdmmc_card_print_info(stdout, _card);
        return true;
    }

    return false;
}

void SDCard::unmount()
{
    esp_vfs_fat_sdmmc_unmount();
}


void SDCard::startLog()
{
    //Create file


    
    //Create task
    xTaskCreate(&sdcard::logTask, "LogTask", 2048, this, 10, &_logTaskHandle);

    //Enable interrupt
    setup_interrupt_pin(true);
}

void SDCard::stopLog()
{

}


bool SDCard::enqueue(imuDataPtr_t data, bool from_isr)
{
    if (data != nullptr && _imuQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_imuQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_imuQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                return true;
            }
        }
    }
    return false;
}

bool SDCard::enqueue(timestampSendable_t data, bool from_isr)
{
    if (_timestampQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_timestampQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_timestampQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                return true;
            }
        }
    }
    return false;
}

//Data from Power
bool SDCard::enqueue(powerDataPtr_t data, bool from_isr)
{
    
    if (data != nullptr && _powerQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_powerQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_powerQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                return true;
            }
        }
    }
    return false;
}


bool SDCard::enqueue(baroDataPtr_t data, bool from_isr)
{
    if (data != nullptr && _baroQueue != nullptr)
    {
        if (from_isr)
        {
            if (xQueueSendFromISR(_baroQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGiveFromISR(_dataReadySemaphore, NULL);
                return true;
            }
        }
        else
        {
            if (xQueueSend(_baroQueue, &data, 0) == pdTRUE)
            {
                xSemaphoreGive(_dataReadySemaphore);
                return true;
            }
        }
    }
    return false;
}