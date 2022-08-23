#ifndef _SDCARD_H_
#define _SDCARD_H_

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>



#define PIN_INTERRUPT_FROM_GPS_REF 32

union timestampSendable_t {
    time_t data;
    uint8_t bytes[sizeof(time_t)];
};

namespace sdcard
{
    void logTask(void *pvParameters);
    void generateTimestamp(void *pvParameters);
}

class SDCard
{
    friend void sdcard::logTask(void *pvParameters);
    friend void generateTimestamp(void *pvParameters);

public:

    static SDCard* instance();

    int toESP32();
    void toExternal();
    bool mount();
    void unmount();

    void startLog();
    void stopLog();
    int getlogID();

    //Data from timestamp (ISR from GPS pulse)
    bool enqueue(timestampSendable_t data, bool from_isr = false);

    //Data from IMU
    bool enqueue(imuDataPtr_t data, bool from_isr = false);

    //Data from Power
    bool enqueue(powerDataPtr_t data, bool from_isr = false);

    //Data from Baro
    bool enqueue(baroDataPtr_t data, bool from_isr = false);

    //Data from GPS
    bool enqueue(gpsDataPtr_t data, bool from_isr = false);

    //Data from Pulse
    bool enqueue(pulseDataPtr_t data, bool from_isr = false);

    bool logFileWrite(const void* data, size_t size);

    bool syncFile();
    //Look in the SD card for the configuration file and takes initial parameters from it
    bool GetIMUConfigFromSd(IMUconfig_Sd *IMUSdConfig);

    bool GetOpenTeraConfigFromSd(OpenTeraConfig_Sd *OpenTeraSdConfig);


    //Check if the SD Card is in
    void checkSD();
    //Return boolean telling if the SD card is present
    bool getSdCardPresent();
    //Tells the space left on the Sd Card
    float getSDfreespace();

protected:

    SemaphoreHandle_t getDataReadySemaphore(){return _dataReadySemaphore;}
    QueueHandle_t getTimestampQueue(){return _timestampQueue;}
    QueueHandle_t getIMUQueue(){return _imuQueue;}
    QueueHandle_t getPowerQueue(){return _powerQueue;}
    QueueHandle_t getBaroQueue(){return _baroQueue;}
    QueueHandle_t getGPSQueue(){return _gpsQueue;}
    QueueHandle_t getPulseQueue(){return _pulseQueue;}

private:
    void setup_gpio(int pin);
    void setup_interrupt_pin(bool enable);

    SDCard();
    virtual ~SDCard();
    void lock(bool from_isr = false);
    void unlock(bool from_isr = false);

    bool SdCardPresent=false;

    static SDCard* _instance;
    sdmmc_host_t _host;
    sdmmc_slot_config_t _slot_config;
    sdmmc_card_t* _card;
    TaskHandle_t _logTaskHandle;
    TaskHandle_t _timestampTask;

    QueueHandle_t _imuQueue;
    QueueHandle_t _gpsQueue;
    QueueHandle_t _powerQueue;
    QueueHandle_t _baroQueue;
    QueueHandle_t _timestampQueue;
    QueueHandle_t _pulseQueue;
    SemaphoreHandle_t _dataReadySemaphore;
    FILE* _logFile;
    SemaphoreHandle_t _mutex;
};



#endif