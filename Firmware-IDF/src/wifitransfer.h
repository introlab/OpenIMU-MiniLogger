

#ifndef _WIFITRANSFER_H
#define _WIFITRANSFER_H

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"

class WiFiTransfer
{
    friend void wifiTransferTask(void *pvParameters);

    public:

    WiFiTransfer();


    //Singleton
    static WiFiTransfer* instance();


    private:

    static WiFiTransfer* _instance;
    TaskHandle_t _wifiTransferTaskHandle;
    SemaphoreHandle_t _mutex;

    void initialize_wifi();

};


#endif // _WIFITRANSFER_H