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

#define PIN_INTERRUPT_FROM_GPS_REF 32

class SDCard
{
public:

    static SDCard* instance();

    void toESP32();
    void toExternal();
    bool mount();
    void unmount();

    void startLog();
    void stopLog();

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

    SemaphoreHandle_t getTimeSemaphore(){return _timeSemaphore;}

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
    SemaphoreHandle_t _timeSemaphore;
};



#endif