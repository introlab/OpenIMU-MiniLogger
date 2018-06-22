#include "gps.h"


namespace
{

    void readGPS(void *pvParameters)
    {
        printf("readGPS starting \n");
        GPS *gps = reinterpret_cast<GPS*>(pvParameters);
        assert(gps != NULL);

        uint8_t buffer[128];

        while(1)
        {
            int len = gps->read_uart(buffer, 128);
            //printf("read len %i\n", len);
            for (int i = 0; i < len; i++)
            {
                printf("%2.2x", buffer[i]);
            }
        }
    }
}

GPS::GPS()
{

    setup_gpio();
    setup_uart();

    xTaskCreate(&readGPS, "ReadGPS", 2048, this, 8, &_readGPSHandle);
    printf("GPS initialized\n");

}

void GPS::setup_gpio()
{
    //RX
    gpio_config_t io_conf_rx;
    //interrupt on falling edge
    io_conf_rx.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
    //set as input mode
    io_conf_rx.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO33
    io_conf_rx.pin_bit_mask =  (1ULL << PIN_NUM_UART_RX);
    //disable pull-down mode
    io_conf_rx.pull_down_en = (gpio_pulldown_t) 0;
    //enable pull-up mode
    io_conf_rx.pull_up_en = (gpio_pullup_t) 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf_rx);

    //TX
    gpio_config_t io_conf_tx;
    //interrupt on falling edge
    io_conf_tx.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
    //set as input mode
    io_conf_tx.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO33
    io_conf_tx.pin_bit_mask =  (1ULL << PIN_NUM_UART_TX);
    //disable pull-down mode
    io_conf_tx.pull_down_en = (gpio_pulldown_t) 0;
    //enable pull-up mode
    io_conf_tx.pull_up_en = (gpio_pullup_t) 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf_tx);


}

void GPS::setup_uart()
{

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    _uart_config.baud_rate = 9600;
    _uart_config.data_bits = UART_DATA_8_BITS;
    _uart_config.parity = UART_PARITY_DISABLE;
    _uart_config.stop_bits = UART_STOP_BITS_1;
    _uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

    uart_param_config(UART_NUM_1, &_uart_config);
    uart_set_pin(UART_NUM_1, PIN_NUM_UART_TX, PIN_NUM_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    esp_err_t err = uart_driver_install(UART_NUM_1, BUFFER_SIZE * 2, 0, 0, NULL, 0);
    if (err == ESP_OK)
        printf("UART Driver installed\n");

}

int GPS::read_uart(uint8_t *buffer, int max_size)
{
    int len = uart_read_bytes(UART_NUM_1, buffer, max_size, 20 / portTICK_RATE_MS);
    return len;
}