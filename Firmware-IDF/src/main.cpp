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
#include "spibus.h"
#include "display.h"

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

namespace Actions
{
    // Software Shutdown by the menu
    void Shutdown()
    {
        
    }

    void SDToESP32()
    {
        
    }

    void SDToExternal()
    {
       
    }

    //Same function to start and stop logging to avoid double start
    void IMUStartSD()     
    {
      

    }

    void IMUStopSD()    // used to be sure to stop log when device shutdown
    {
       
    }
}

void ledBlink(void *pvParameters)
{
  while(1)
  {
    IOExpander::instance().digitalWrite(EXT_PIN01_LED, LOW);

    vTaskDelay(500 / portTICK_RATE_MS);

    IOExpander::instance().digitalWrite(EXT_PIN01_LED, HIGH);

    vTaskDelay(500 / portTICK_RATE_MS);
  }
}


//app_main should have a "C" signature
extern "C" 
{
    void app_main()
    {
        esp_err_t ret;

        //SPI bus configuration
        SPIBus spibus;

        //I2C bus configuration
        i2c_config_t i2ccfg;
        ret = configure_i2c(i2ccfg);

        vTaskDelay(500 / portTICK_RATE_MS);

        //install gpio isr service
        gpio_install_isr_service(0);

        //Get single instance of IOExpander...
        IOExpander &ioExpander = IOExpander::instance();

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

        TaskHandle_t ledBlinkHandle;
        xTaskCreatePinnedToCore(&ledBlink, "Blinky", 2048, NULL, 1, &ledBlinkHandle,1);

        ADC adc(I2C_NUM_1);

        Display display;
        display.begin();
        display.clear();
        //display.showSplashScreen(0);

        //Do better...
        while(1)
        {
            //Buttons tests
            printf("B0: %i\n", ioExpander.digitalRead(EXT_PIN11_BUTTON0));
            printf("B1: %i\n", ioExpander.digitalRead(EXT_PIN06_BUTTON1));
            printf("B2: %i\n", ioExpander.digitalRead(EXT_PIN08_BUTTON2));
            printf("B3: %i\n", ioExpander.digitalRead(EXT_PIN09_BUTTON3));

            //ADC tests
            printf("Batt: %4.4f Current: %4.4f\n", adc.read_voltage(), adc.read_current());

            display.displayVoltage(adc.read_voltage(), adc.read_current(), false, false, false);
            vTaskDelay(100 / portTICK_RATE_MS);
        }
    }
}