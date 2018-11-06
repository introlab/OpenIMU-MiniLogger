#include "bluetooth.h"

//Static instance
Bluetooth* Bluetooth::_instance = NULL;
// char* Bluetooth::DEMO_TAG = (char*)"EDDYSTONE_DEMO";
esp_ble_scan_params_t Bluetooth::ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    // .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ONLY_WLST,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x400,
    .scan_window            = 0x320,
    .scan_duplicate         = BLE_SCAN_DUPLICATE_ENABLE
};

namespace bluetooth
{
    void receiveTask(void *pvParameters)
    {
        Bluetooth *bluetooth = reinterpret_cast<Bluetooth*>(pvParameters);
        assert(bluetooth);
        uint32_t duration = 1; // scanning duration in sec
        TickType_t lastGeneration = xTaskGetTickCount();
        while(1){
            // every 20 sec, check eddystone ble device
            vTaskDelayUntil(&lastGeneration, 20000 / portTICK_RATE_MS);
            printf("Bluetooth task\n");
            esp_ble_gap_start_scanning(duration);
            
        }
    }
}

Bluetooth* Bluetooth::instance()
{
    if (Bluetooth::_instance == NULL)
        Bluetooth::_instance = new Bluetooth();
    return Bluetooth::_instance;
}

Bluetooth::Bluetooth()
    :   _receivTaskHandle(NULL)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);

    esp_bluedroid_init();
    esp_bluedroid_enable();
    esp_err_t status;
    
    printf("\nBuetooth Init : Register callback\n");
    /*<! register the scan callback function to the gap module */
    if((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK) {
        printf("gap register error: %x", status);
    }
    
    //  Set scan params launch continuous scan
    esp_ble_gap_set_scan_params(&ble_scan_params);
    esp_ble_gap_stop_scanning();

    start();

}

Bluetooth::~Bluetooth()
{

}

void Bluetooth::start()
{

    
xTaskCreatePinnedToCore(&bluetooth::receiveTask, "BLE Tag", 2048, this, 15, &_receivTaskHandle, 1);

}

void Bluetooth::esp_eddystone_show_inform(const esp_eddystone_result_t* res){
    switch(res->common.frame_type)
    {
        case EDDYSTONE_FRAME_TYPE_UID: {
            printf("Eddystone UID inform\n");
            printf("Measured power(RSSI at 0m distance) :%d dbm\nNamespaceID : ", res->inform.uid.ranging_data);
            for (int i = 0; i < 10; i++) { printf("%x.",res->inform.uid.namespace_id[i]); }
            printf("\nInstanceID : ");
            for (int i = 0; i < 6; i++) { printf("%x.",res->inform.uid.instance_id[i]); }
            printf("\n");
            break;
        }
        case EDDYSTONE_FRAME_TYPE_URL: {
            printf("Eddystone URL inform\n");            
            printf("Measured power(RSSI at 0m distance) :%d dbm\n", res->inform.url.tx_power);
            printf("URL: %s\n",res->inform.url.url);
            break;
        }
        case EDDYSTONE_FRAME_TYPE_TLM: {
            printf("Eddystone TLM inform\n");
            printf("version: %d", res->inform.tlm.version);
            break;
        }
        default:
            break;
    }
}

void Bluetooth::esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param)
{
    esp_err_t err;

    switch(event)
    {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
            uint32_t duration = 0;
            esp_ble_gap_start_scanning(duration);
            break;
        }
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: {
            if((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
                printf("Scan start failed: %x", err);
            }
            else {
                printf("Start scanning...");
            }
            break;
        }
        case ESP_GAP_BLE_SCAN_RESULT_EVT: {
            esp_ble_gap_cb_param_t* scan_result = (esp_ble_gap_cb_param_t*)param;
            switch(scan_result->scan_rst.search_evt)
            {
                case ESP_GAP_SEARCH_INQ_RES_EVT: {
                    esp_eddystone_result_t eddystone_res;
                    memset(&eddystone_res, 0, sizeof(eddystone_res));
                    esp_err_t ret = esp_eddystone_decode(scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len, &eddystone_res);
                    if (ret) {
                        // error:The received data is not an eddystone frame packet or a correct eddystone frame packet.
                        // just return
                        return;
                    } else {   
                        // The received adv data is a correct eddystone frame packet.
                        // Here, we get the eddystone infomation in eddystone_res, we can use the data in res to do other things.
                        // For example, just print them:
                        printf("Eddystone Found\n");
                        esp_log_buffer_hex("Device address:", scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
                        // printf("RSSI of packet:%d dbm", scan_result->scan_rst.rssi);
                        esp_eddystone_show_inform(&eddystone_res);    
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:{
            if((err = param->scan_stop_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
                printf("Scan stop fail");
            }
            else {
                printf("Scan stop succes");
            }
            break;
        }
        default:
            break;
    }
}