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
}

class SDCard
{
    friend void sdcard::logTask(void *pvParameters);

public:

    static SDCard* instance();

    void toESP32();
    void toExternal();
    bool mount();
    void unmount();

    void startLog();
    void stopLog();

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
    
    bool logFileWrite(const void* data, size_t size);

    bool syncFile();

    /* 
    void begin();

    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
    bool mount();

    
    

    void setDataReadySemaphore(SemaphoreHandle_t semaphore);
    void setIMUQueue(QueueHandle_t queue);
    void setGPSQueue(QueueHandle_t queue);
    void setPowerQueue(QueueHandle_t queue);
    void setBarometerQueue(QueueHandle_t queue);

    void startLog();
    void stopLog();

private:
    void startTimestamp();
    void stopTimestamp();

    */
protected:

    SemaphoreHandle_t getDataReadySemaphore(){return _dataReadySemaphore;}
    QueueHandle_t getTimestampQueue(){return _timestampQueue;}
    QueueHandle_t getIMUQueue(){return _imuQueue;}
    QueueHandle_t getPowerQueue(){return _powerQueue;}
    QueueHandle_t getBaroQueue(){return _baroQueue;}
    QueueHandle_t getGPSQueue(){return _gpsQueue;}

private:
    void setup_gpio(int pin);
    void setup_interrupt_pin(bool enable);

    SDCard();
    virtual ~SDCard();
    static SDCard* _instance;
    sdmmc_host_t _host;
    sdmmc_slot_config_t _slot_config;
    sdmmc_card_t* _card;
    TaskHandle_t _logTaskHandle;

    QueueHandle_t _imuQueue; 
    QueueHandle_t _gpsQueue;
    QueueHandle_t _powerQueue;
    QueueHandle_t _baroQueue; 
    QueueHandle_t _timestampQueue; 
    SemaphoreHandle_t _dataReadySemaphore; 
    FILE* _logFile;
};



#endif