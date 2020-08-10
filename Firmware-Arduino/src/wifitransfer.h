

#ifndef _WIFITRANSFER_H
#define _WIFITRANSFER_H

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include <stdio.h>
#include <string.h>
#include <list>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"

class WiFiTransfer
{
    friend void wifiTransferTask(void *pvParameters);

    public:

    typedef enum {
        STATE_DISCONNECTED, 
        STATE_CONNECTED, 
        STATE_LOGIN,
        STATE_LOGIN_ERROR, 
        STATE_CREATE_SESSION,
        STATE_CREATE_SESSION_ERROR,
        STATE_UPLOAD_FILE,
        STATE_UPLOAD_FILE_ERROR,
        STATE_DONE, 
        STATE_ERROR } wifi_state;


    WiFiTransfer();

    //Singleton
    static WiFiTransfer* instance();
    void start_transfer();
    void stop_transfer();
    void initialize_wifi();
    void terminate_wifi();
    void lock();
    void unlock();
    void setState(wifi_state state);
    WiFiTransfer::wifi_state getState();
    std::string getStateString();

    protected:

    static WiFiTransfer* _instance;
    TaskHandle_t _wifiTransferTaskHandle;
    SemaphoreHandle_t _mutex;
    WiFiTransfer::wifi_state _state;

};


#endif // _WIFITRANSFER_H