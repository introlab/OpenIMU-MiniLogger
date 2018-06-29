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
#include "power.h"
#include "spibus.h"
#include "i2cbus.h"
#include "display.h"
#include "imu.h"
#include "sdcard.h"
#include "barometer.h"
#include "gps.h"
#include "buttons.h"


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

        //This needs to be called first
        //install gpio isr service
        gpio_install_isr_service(0);

        //SPI bus configuration
        SPIBus spibus;

        //I2C bus configuration
        I2CBus i2cbus;

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


        TaskHandle_t ledBlinkHandle;
        xTaskCreate(&ledBlink, "Blinky", 2048, NULL, 1, &ledBlinkHandle);

        Buttons *buttons = Buttons::instance();
        assert(buttons);


        SDCard *sdcard = SDCard::instance();
        assert(sdcard);

        IMU *imu = IMU::instance();
        assert(imu);

        Power *power = Power::instance();
        assert(power);

        Barometer *baro = Barometer::instance();
        assert(baro);

        Display *display = Display::instance();
        assert(display);

        GPS*  gps = GPS::instance();
        assert(gps);

        //Debug
        SDCard::instance()->startLog();

        Menu menu;


        display->begin();
        display->clear();
        //display.showSplashScreen(0);

        int change_counter = 0;

        //Do better...
        while(1)
        {
            bool changed = false;

            while(buttons->getActionCtn() > 0) 
            {
                menu.action();
                buttons->decrementActionCtn();
                changed = true;
            }

            while(buttons->getPreviousCtn() > 0) 
            {
                menu.previous();
                buttons->decrementPreviousCtn();
                changed = true;
            }

            while(buttons->getNextCtn() > 0) 
            {
                menu.next();
                buttons->decrementNextCtn();
                changed = true;
            }

            while(buttons->getBackCtn() > 0) 
            {
                display->displayVoltage(0, 0, 0, false, false);
                buttons->decrementBackCtn();
            }
            
            if(changed) 
            {
            
                display->updateMenu(&menu,false);
            
                change_counter = 0;
            }
            else 
            {
                change_counter++;

                // Every 5 seconds verify if no activity, then paint state
                if (change_counter > 50)
                {
                    display->displayVoltage(0, 0 , false, false, false);
                    change_counter = 0;
                }
            }
        
            // Sleep for 100ms
            vTaskDelay(100 / portTICK_RATE_MS);
        } //while
    }
}
