#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "driver/i2c.h"
#include "ioexpander.h"
#include "adc.h"

esp_err_t configure_i2c(i2c_config_t &conf)
{
    i2c_port_t i2c_master_port = I2C_NUM_1;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t) PIN_NUM_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t) PIN_NUM_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    i2c_param_config(i2c_master_port, &conf);

    //Master mode does not need buffers
    esp_err_t ret = i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
    printf("configure_i2c ret: %i\n", ret);
    assert(ret == ESP_OK);

    return ret;
}

esp_err_t configure_spi(spi_bus_config_t &buscfg)
{
        buscfg.miso_io_num = PIN_NUM_MISO;
        buscfg.mosi_io_num = PIN_NUM_MOSI;
        buscfg.sclk_io_num = PIN_NUM_CLK;
        buscfg.quadwp_io_num = -1;
        buscfg.quadhd_io_num = -1;
        buscfg.max_transfer_sz = 1000;
  
        //Needed for adequate SPI pin configuration
        gpio_set_direction((gpio_num_t)PIN_NUM_MISO, GPIO_MODE_INPUT);


        //Initialize the SPI bus data structure, no DMA for now
        esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);
        printf("configure_spi ret: %i\n", ret);
        assert(ret == ESP_OK);

        return ret;
}


//app_main should have a "C" signature
extern "C" 
{
    void app_main()
    {
        esp_err_t ret;

        //SPI bus configuration
        spi_bus_config_t spicfg;
        ret = configure_spi(spicfg);

        //I2C bus configuration
        i2c_config_t i2ccfg;
        ret = configure_i2c(i2ccfg);

        vTaskDelay(500 / portTICK_RATE_MS);

        IOExpander ioExpander(0, (gpio_num_t) 5, HSPI_HOST);

        //ALIVE -->HIGH, power will stay on
        ioExpander.pinMode(EXT_PIN12_KEEP_ALIVE, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN12_KEEP_ALIVE, HIGH);

        //LED
        ioExpander.pinMode(EXT_PIN01_LED, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN01_LED, HIGH);

        //BATT READ ENABLE
        ioExpander.pinMode(EXT_PIN13_BATT_READ_EN, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN13_BATT_READ_EN, HIGH);


        //Buttons
        ioExpander.pinMode(EXT_PIN11_BUTTON0, INPUT);
        ioExpander.pullupMode(EXT_PIN11_BUTTON0, HIGH);
        ioExpander.pinMode(EXT_PIN06_BUTTON1, INPUT);
        ioExpander.pullupMode(EXT_PIN06_BUTTON1, HIGH);
        ioExpander.pinMode(EXT_PIN08_BUTTON2, INPUT);
        ioExpander.pullupMode(EXT_PIN08_BUTTON2, HIGH);
        ioExpander.pinMode(EXT_PIN09_BUTTON3, INPUT);
        ioExpander.pullupMode(EXT_PIN09_BUTTON3, HIGH);


        ADC adc(I2C_NUM_1);

        //Do better...
        while(1)
        {
            //Buttons tests
            printf("B0: %i\n", ioExpander.digitalRead(EXT_PIN11_BUTTON0));
            printf("B1: %i\n", ioExpander.digitalRead(EXT_PIN06_BUTTON1));
            printf("B2: %i\n", ioExpander.digitalRead(EXT_PIN08_BUTTON2));
            printf("B3: %i\n", ioExpander.digitalRead(EXT_PIN09_BUTTON3));

            //ADC tests
            printf("ADC0: %i\n", adc.readADC_SingleEnded(0));
            printf("ADC1: %i\n", adc.readADC_SingleEnded(1));
            printf("ADC2: %i\n", adc.readADC_SingleEnded(2));
            printf("ADC3: %i\n", adc.readADC_SingleEnded(3));


            ioExpander.digitalWrite(EXT_PIN01_LED, HIGH); 
            vTaskDelay(1000 / portTICK_RATE_MS);
            ioExpander.digitalWrite(EXT_PIN01_LED, LOW);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    }
}