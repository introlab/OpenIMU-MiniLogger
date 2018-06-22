#include "gps.h"
#include <string.h>

namespace
{

    void readGPS(void *pvParameters)
    {
        printf("readGPS starting \n");
        GPS *gps = reinterpret_cast<GPS*>(pvParameters);
        assert(gps != NULL);

        
        uint8_t buffer[128];
        memset(buffer, 0, 128 * sizeof(uint8_t));

        while(1)
        {
            memset(buffer, 0, 128 * sizeof(uint8_t));
            int len = gps->read_uart(buffer, 128);
            //printf("read len %i\n", len);
            printf("%s", buffer);
            
        }
    }
}

GPS::GPS()
    : _port(UART_NUM_1)
{
    setup_uart();
    
    xTaskCreate(&readGPS, "ReadGPS", 2048, this, 8, &_readGPSHandle);
    printf("GPS initialized\n");

}

void GPS::setup_uart()
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .use_ref_tick = 1
    };
    uart_param_config(_port, &uart_config);

    uart_set_pin(_port, PIN_NUM_UART_TX, PIN_NUM_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    esp_err_t err = uart_driver_install(_port, BUFFER_SIZE * 2, 0, 0, NULL, 0);
    if (err == ESP_OK)
        printf("UART Driver installed\n");

}

int GPS::read_uart(uint8_t *buffer, int max_size)
{
    int len = uart_read_bytes(_port, buffer, max_size, 20 / portTICK_RATE_MS);
    return len;
}