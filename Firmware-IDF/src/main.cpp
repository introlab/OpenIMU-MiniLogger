#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "ioexpander.h"

//app_main should have a "C" signature
extern "C" 
{
 

    void app_main()
    {
        esp_err_t ret;



        //SPI bus configuration
        spi_bus_config_t buscfg;
        buscfg.miso_io_num = PIN_NUM_MISO;
        buscfg.mosi_io_num = PIN_NUM_MOSI;
        buscfg.sclk_io_num = PIN_NUM_CLK;
        buscfg.quadwp_io_num = -1;
        buscfg.quadhd_io_num = -1;
        buscfg.max_transfer_sz = 1000;
  
        gpio_set_direction((gpio_num_t)PIN_NUM_MISO, GPIO_MODE_INPUT);


        //Initialize the SPI bus data structure, no DMA for now
        ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);
        printf("SPI BUS RET : %i\n", ret);
        assert(ret == ESP_OK);

        vTaskDelay(500 / portTICK_RATE_MS);

        IOExpander ioExpander(0, (gpio_num_t) 5, HSPI_HOST);

        //ALIVE -->HIGH, power will stay on
        ioExpander.pinMode(EXT_PIN12_KEEP_ALIVE, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN12_KEEP_ALIVE, HIGH);

        //LED
        ioExpander.pinMode(EXT_PIN01_LED, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN01_LED, HIGH);

        //Buttons
        ioExpander.pinMode(EXT_PIN11_BUTTON0, INPUT);
        ioExpander.pullupMode(EXT_PIN11_BUTTON0, HIGH);
        ioExpander.pinMode(EXT_PIN06_BUTTON1, INPUT);
        ioExpander.pullupMode(EXT_PIN06_BUTTON1, HIGH);
        ioExpander.pinMode(EXT_PIN08_BUTTON2, INPUT);
        ioExpander.pullupMode(EXT_PIN08_BUTTON2, HIGH);
        ioExpander.pinMode(EXT_PIN09_BUTTON3, INPUT);
        ioExpander.pullupMode(EXT_PIN09_BUTTON3, HIGH);

        //Do better...
        while(1)
        {
            printf("B0: %i\n", ioExpander.digitalRead(EXT_PIN11_BUTTON0));
            printf("B1: %i\n", ioExpander.digitalRead(EXT_PIN06_BUTTON1));
            printf("B2: %i\n", ioExpander.digitalRead(EXT_PIN08_BUTTON2));
            printf("B3: %i\n", ioExpander.digitalRead(EXT_PIN09_BUTTON3));
            ioExpander.digitalWrite(EXT_PIN01_LED, HIGH); 
            vTaskDelay(1000 / portTICK_RATE_MS);
            ioExpander.digitalWrite(EXT_PIN01_LED, LOW);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    }
}