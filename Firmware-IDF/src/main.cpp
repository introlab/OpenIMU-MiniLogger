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
#include <soc/efuse_reg.h>

#include "ioexpander.h"
#include "power.h"
#include "spibus.h"
#include "i2cbus.h"
#include "display.h"
#include "imu.h"
#include "sdcard.h"
#include "barometer.h"
#include "gps.h"
#include "pulse.h"
#include "bluetooth.h"
#include "buttons.h"
#include "widget/widget.h"
#include "widget/battery.h"
#include "widget/gps.h"
#include "widget/log.h"
#include "widget/sd.h"
#include "homescreen.h"

#include <SparkFunMPU9250-DMP.h>

namespace Actions
{
    bool loggingEnabled = false;
    bool wasLogging = false;
    bool sdcardExternal = false;

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

    void ToggleSD()
    {
        if (sdcardExternal)
        {
            SDToESP32();
        }
        else
        {
            SDToExternal();
        }
        
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
    void test_loop()
    {
        //Get single instance of IOExpander...
        IOExpander &ioExpander = IOExpander::instance();

        Buttons *buttons = Buttons::instance();
        assert(buttons);

        Power *power = Power::instance();
        assert(power);

        bool state = false;
        int count = 0;

        while(1)
        {
            vTaskDelay(100 / portTICK_RATE_MS);

            float voltage = power->last_voltage();
            float current = power->last_current();

            printf("%f %f \n", voltage, current);
            

            printf("BackCnt %i \n", buttons->getBackCtn());
            if (buttons->getBackCtn() > 0)
            {
                buttons->decrementBackCtn();
                

                if (state)
                {
                    state = false;
                    ioExpander.digitalWrite(EXT_PIN15_MOTOR_VIBRATE, LOW);
                }
                else
                {
                    state = true;
                    ioExpander.digitalWrite(EXT_PIN15_MOTOR_VIBRATE, HIGH);
                }

                    
                

                printf("Button Pressed \n");
            }

            printf("Counter %i \n", count++);



  
        }
    }


    void app_main()
    {
        //This needs to be called first
        //install gpio isr service
        gpio_install_isr_service(0);

        //Set hardcoded timezone for now, TODO move somwhere else?
        setenv("TZ", "GEST+5EDT,M3.2.0/2,M11.1.0/2", 1);
        tzset();

        esp_err_t ret=0;

        //SPI bus configuration
        SPIBus spibus;

        //I2C bus configuration
        I2CBus i2cbus;

        //I2C Ext bus configuration
        I2CBusExt i2cbusext;



        MPU9250_DMP dmp;
        dmp.begin();
        dmp.update();


        //Get single instance of IOExpander...
        IOExpander &ioExpander = IOExpander::instance();

        //LED
        ioExpander.pinMode(EXT_PIN01_LED, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN01_LED, HIGH);

        //BATT READ ENABLE
        ioExpander.pinMode(EXT_PIN13_BATT_READ_EN, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN13_BATT_READ_EN, HIGH);

        //MOTOR VIBRATION
        ioExpander.pinMode(EXT_PIN15_MOTOR_VIBRATE, OUTPUT);
        ioExpander.digitalWrite(EXT_PIN15_MOTOR_VIBRATE, LOW);

        //ENABLE PROGRAMMING
        gpio_pad_select_gpio(PIN_NUM_ENABLE_PROGRAMMING);
        gpio_set_direction((gpio_num_t)PIN_NUM_ENABLE_PROGRAMMING, GPIO_MODE_OUTPUT);

        //PIN_NUM_ENABLE_PROGRAMMING = 0 ---> CAN PROGRAM
        gpio_set_level((gpio_num_t)PIN_NUM_ENABLE_PROGRAMMING, 0);

        //Display
        Display *display = Display::instance();
        assert(display);
        display->begin();
        display->showSplashScreen(0);
        TickType_t splashTime = xTaskGetTickCount();

        TaskHandle_t ledBlinkHandle;
        xTaskCreate(&ledBlink, "Blinky", 2048, NULL, 1, &ledBlinkHandle);

        Buttons *buttons = Buttons::instance();
        assert(buttons);

        //DL - Testing serial port fix
        //test_loop();
        SDCard *sdcard = SDCard::instance();
        assert(sdcard);

        //IMU *imu = IMU::instance();
        //assert(imu);


        Power *power = Power::instance();
        assert(power);

        GPS*  gps = GPS::instance();
        assert(gps);

       
        Barometer *baro = Barometer::instance();
        assert(baro);

        //Pulse should be started from menu?
        //Not yet working...
        //Pulse *pulse = Pulse::instance();
        //assert(pulse);

        //Bluetooth *ble = Bluetooth::instance();
        //assert(ble);

        // HOMESCREEN

        Widget::Battery batteryWidget;
        batteryWidget.updateValue(5.0, 0.0, true);

        Widget::GPS gpsWidget;
        gpsWidget.setStatus(false);

        Widget::Log logWidget(Actions::IMUStartSD);
        logWidget.setStatus(false);

        Widget::SD sdWidget(Actions::ToggleSD);
        sdWidget.setStatus(false);

        Homescreen home;
        home.addWidget(&batteryWidget);
        home.addWidget(&gpsWidget);
        home.addWidget(&logWidget);
        home.addWidget(&sdWidget);

        // Show homescreen and disable programming
        vTaskDelayUntil(&splashTime, 4200 / portTICK_RATE_MS);
        ioExpander.digitalWrite(EXT_PIN15_MOTOR_VIBRATE, HIGH);
        vTaskDelay(400 / portTICK_RATE_MS);
        ioExpander.digitalWrite(EXT_PIN15_MOTOR_VIBRATE, LOW);
        //PIN_NUM_ENABLE_PROGRAMMING = 0 ---> CAN PROGRAM
        //gpio_set_level((gpio_num_t)PIN_NUM_ENABLE_PROGRAMMING, 0);
        home.setVisible(true);

        //Debug
        //Actions::IMUStartSD();
        //Do better...
        TickType_t lastBtn = xTaskGetTickCount();
        TickType_t lastRefresh = xTaskGetTickCount();
        TickType_t now;
        bool active = true;

        while(1)
        {
            //printf("%f %f \n", voltage, current);

            // Sleep for 100ms
            vTaskDelay(100 / portTICK_RATE_MS);
           
           // Check buttons
            while(buttons->getActionCtn() > 0) 
            {
                if (active) home.action();
                buttons->decrementActionCtn();
                lastBtn = xTaskGetTickCount();
            }

            while(buttons->getPreviousCtn() > 0) 
            {
                if (active) home.previous();
                buttons->decrementPreviousCtn();
                lastBtn = xTaskGetTickCount();
            }

            while(buttons->getNextCtn() > 0) 
            {
                if (active) home.next();
                buttons->decrementNextCtn();
                lastBtn = xTaskGetTickCount();
            }

            while(buttons->getBackCtn() > 0) 
            {
                buttons->decrementBackCtn();
                lastBtn = xTaskGetTickCount();
            }

            // Update widgets
            batteryWidget.updateValue(power->last_voltage(), power->last_current(), power->last_charging());
            gpsWidget.setStatus(gps->getFix());
            logWidget.setStatus(Actions::loggingEnabled);
            sdWidget.setStatus(Actions::sdcardExternal);

            if (Actions::loggingEnabled && !Actions::wasLogging)
            {
                home.startLog(-1);
            }
            else if (!Actions::loggingEnabled && Actions::wasLogging)
            {
                home.stopLog();
            }
            Actions::wasLogging = Actions::loggingEnabled;

            // Check activity
            now = xTaskGetTickCount();
            if (now-lastBtn > SCREEN_SLEEP_TIMER/portTICK_RATE_MS)
            {
                display->setBrightness(Display::Brigthness::SLEEP);
                active = false;
            }
            else if (now-lastBtn > SCREEN_DIM_TIMER/portTICK_RATE_MS)
            {
                if (active) display->setBrightness(Display::Brigthness::DIM);
                active = false;
            }
            else {
                if (!active) display->setBrightness(Display::Brigthness::NORMAL);
                active = true;
            }

            // Update time each second
            if (now-lastRefresh > 1000/portTICK_RATE_MS)
            {
                home.setVisible(true);
                lastRefresh = now;
            }
        } //while 
    }
}
