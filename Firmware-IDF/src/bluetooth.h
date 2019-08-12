#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "string.h"
#include "esp_bt.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_defs.h"
#include "esp_gattc_api.h"
#include "esp_gap_ble_api.h"

#include "esp_eddystone_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_eddystone_api.h"

#ifdef __cplusplus
}
#endif

namespace bluetooth
{
    void receiveTask(void *pvParameters);
}

class Bluetooth
{
    friend void receiveTask(void *pvParameters);
public:
    static Bluetooth* instance();

    void start();
    static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
    static void esp_eddystone_show_inform(const esp_eddystone_result_t* res);


private:
    Bluetooth();
    virtual ~Bluetooth();

    static Bluetooth* _instance;
    TaskHandle_t _receivTaskHandle;
    static esp_ble_scan_params_t ble_scan_params;

};



#endif