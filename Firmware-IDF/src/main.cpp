#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include <soc/efuse_reg.h>


#include "ioexpander.h"
#include "power.h"
#include "spibus.h"
#include "i2cbus.h"
#include "i2cbusext.h"
#include "display.h"
#include "imu.h"
#include "sdcard.h"
#include "barometer.h"
#include "pulse.h"
#include "gps.h"
#include "buttons.h"

#include "bluetooth.h"


namespace Actions
{
    bool loggingEnabled = false;
    bool sdcardExternal = false;

    // Software Shutdown by the menu
    void Shutdown()
    {
        if (loggingEnabled)
        {
            loggingEnabled = false;
            SDCard::instance()->stopLog();  
        }
        uint64_t mac_adress = 0;
        esp_efuse_mac_get_default((uint8_t*) (&mac_adress));

        //Show spash screen with mac
        Display::instance()->showSplashScreen(mac_adress);

        //Shutdown
        printf("Bye!\n");
        IOExpander::instance().digitalWrite(EXT_PIN12_KEEP_ALIVE, LOW);

    }

    void SDToESP32()
    {
        if (loggingEnabled)
        {
            loggingEnabled = false;
            SDCard::instance()->stopLog();  
        }
        sdcardExternal = false;
        SDCard::instance()->toESP32();
    }

    void SDToExternal()
    {
        if (loggingEnabled)
        {
            loggingEnabled = false;
            SDCard::instance()->stopLog();  
        }
        sdcardExternal = true;
        SDCard::instance()->toExternal();
    }

    void LaunchMAX()
    {
            
        if (loggingEnabled)
        {
            loggingEnabled = false;
            SDCard::instance()->stopLog();  
        }
        sdcardExternal = true;
        Pulse::instance()->connect();
    
    }

    //Same function to start and stop logging to avoid double start
    void IMUStartSD()
    {
        printf("IMUStartSD\n");
        if (loggingEnabled)
        {
            printf("Stopping log\n");
            loggingEnabled = false;
            sdcardExternal = false;
            SDCard::instance()->stopLog();
        }
        else
        {
            printf("Starting log\n");
            loggingEnabled = true;
            sdcardExternal = false;
            SDCard::instance()->startLog();
        }
    }
}

void ledBlink(void *pvParameters)
{

    //Initialize last tick
    TickType_t _lastTick = xTaskGetTickCount();

    int state = LOW;

    while(1)
    {
        //2Hz
        vTaskDelayUntil(&_lastTick, 500 / portTICK_RATE_MS);
        IOExpander::instance().digitalWrite(EXT_PIN01_LED, state);
        state = !state;
        
    }
}

//app_main should have a "C" signature
extern "C"
{
    void app_main()
    {
        // esp_err_t ret;

        //This needs to be called first
        //install gpio isr service
        gpio_install_isr_service(0);

        //SPI bus configuration
        SPIBus spibus;

        //I2C bus configuration
        I2CBus i2cbus;

        //I2C Ext bus configuration
        I2CBusExt i2cbusext;

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

        GPS*  gps = GPS::instance();
        assert(gps);

        IMU *imu = IMU::instance();
        assert(imu);

        Power *power = Power::instance();
        assert(power);

        //Enable External Power, Low State to pilot the transistor
        ioExpander.pinMode(EXT_PIN14_EXTERNAL_POWER_EN, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN14_EXTERNAL_POWER_EN,LOW);
        
        Barometer *baro = Barometer::instance();
        assert(baro);

        Pulse *pulse = Pulse::instance();
        assert(pulse);

        Display *display = Display::instance();
        assert(display);

        Bluetooth *bluetooth = Bluetooth::instance();
        assert(bluetooth);


        Menu menu;

        display->begin();
        display->clear();
    
        int change_counter = 0;

        //Debug
        //Actions::IMUStartSD();
        //Do better...
        while(1)
        {
            bool changed = false;
            float voltage = power->read_voltage();
            float current = power->read_current();

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
                // display->displayVoltage(voltage, current, true, Actions::loggingEnabled, Actions::sdcardExternal);
                display->displayVoltage(voltage, current, gps->getFix(), Actions::loggingEnabled, Actions::sdcardExternal,pulse->get_plugged());
                buttons->decrementBackCtn();
            }
            
            if(changed) 
            {
                display->updateMenu(&menu, Actions::loggingEnabled);
                change_counter = 0;
            }
            else 
            {
                change_counter++;

                // Every 5 seconds verify if no activity, then paint state
                if (change_counter > 50)
                {
                    // display->displayVoltage(voltage, current , true, Actions::loggingEnabled, Actions::sdcardExternal);
                    display->displayVoltage(voltage, current , gps->getFix(), Actions::loggingEnabled, Actions::sdcardExternal,pulse->get_plugged());
                    change_counter = 0;
                }
            }
        
            // Sleep for 100ms
            vTaskDelay(100 / portTICK_RATE_MS);
        } //while
    }
}