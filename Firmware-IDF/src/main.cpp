#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
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
#include "widget/samplerate.h"
#include "widget/gyrorange.h"
#include "widget/accelrange.h"
#include "widget/SDfreespace.h"
#include "widget/wifitransfer.h"
#include "homescreen.h"
#include "configmanager.h"

//Prototype wifi transfer
#include "wifitransfer.h"

namespace Actions
{
    bool loggingEnabled = false;
    bool wasLogging = false;
    bool sdcardExternal = false;
    int SampleRateCounter = 1;
    int GyroRangeCounter = 1;
    int AccelRangeCounter = 1;

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
        if (SDCard::instance()->getSdCardPresent())
        {
            if (loggingEnabled)
            {
                loggingEnabled = false;
                SDCard::instance()->stopLog();  
            }
            sdcardExternal = true;
            SDCard::instance()->toExternal();
        }
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
        if (loggingEnabled )
        {
            printf("Stopping log\n");
            loggingEnabled = false;
            sdcardExternal = false;
            SDCard::instance()->stopLog();
        }
        else if (SDCard::instance()->getSdCardPresent())
        {
            printf("Starting log\n");
            loggingEnabled = true;
            sdcardExternal = false;
            SDCard::instance()->startLog();
        }
        else
        {
            printf("No Sd Card found\n");
        }
        
    }
    //Fonction to Change the sample rate of the IMU
    void ChangeSampleRate()
    {   
        if(!loggingEnabled)
        {
            //Get actual configuration
            IMUconfig_Sd config = ConfigManager::instance()->getIMUConfig();

            if(SampleRateCounter==1)
            {
                config.IMUSampleRate = 50;
                IMU::instance()->setSampleRate(50);
                SampleRateCounter++;
            }
            else if(SampleRateCounter==2)
            {
                config.IMUSampleRate = 100;
                IMU::instance()->setSampleRate(100);
                SampleRateCounter++;
            }
            else if(SampleRateCounter==3)
            {
                config.IMUSampleRate = 200;
                IMU::instance()->setSampleRate(200);
                SampleRateCounter++;
            }
            else if(SampleRateCounter==4)
            {
                config.IMUSampleRate = 10;
                IMU::instance()->setSampleRate(10);
                SampleRateCounter=1;
            }

            //Update configuration
            ConfigManager::instance()->setIMUConfig(config);

            //Save config
           
            if (ConfigManager::instance()->save_configuration())
            {
                printf("Configuration saved!\n");
            }
            else
            {
                printf("Error! Configuration not saved!\n");
            }
        }
        else
        {
            printf("Can't change sample rate while logging\n");
        }
        
    }

    //Fonction to Change the Gyroscope Range of the IMU
    void ChangeGyroRange()
    {   
        if(!loggingEnabled)
        {
            //Get actual configuration
            IMUconfig_Sd config = ConfigManager::instance()->getIMUConfig();

            if(GyroRangeCounter==1)
            {
                config.IMUGyroRange = 500;
                IMU::instance()->setGyroRange(500);
                GyroRangeCounter++;
            }
            else if(GyroRangeCounter==2)
            {
                config.IMUGyroRange = 1000;
                IMU::instance()->setGyroRange(1000);
                GyroRangeCounter++;
            }
            else if(GyroRangeCounter==3)
            {
                config.IMUGyroRange = 2000;
                IMU::instance()->setGyroRange(2000);
                GyroRangeCounter++;
            }
            else if(GyroRangeCounter==4)
            {
                config.IMUGyroRange = 250;
                IMU::instance()->setGyroRange(250);
                GyroRangeCounter=1;
            }

            //Update configuration
            ConfigManager::instance()->setIMUConfig(config);

            //Save config
            if (ConfigManager::instance()->save_configuration())
            {
                printf("Configuration saved!\n");
            }
            else
            {
                printf("Error! Configuration not saved!\n");
            }
        }
        else
        {
            printf("Can't change gyroscope range while logging\n");
        }
        
    }

    //Fonction to Change the Accelerometer Range of the IMU
    void ChangeAccelRange()
    {   
        if(!loggingEnabled)
        {
            //Get actual configuration
            IMUconfig_Sd config = ConfigManager::instance()->getIMUConfig();

            if(AccelRangeCounter==1)
            {
                config.IMUAcellRange = 4;
                IMU::instance()->setAccelRange(4);
                AccelRangeCounter++;
            }
            else if(AccelRangeCounter==2)
            {
                config.IMUAcellRange = 8;
                IMU::instance()->setAccelRange(8);
                AccelRangeCounter++;
            }
            else if(AccelRangeCounter==3)
            {
                config.IMUAcellRange = 16;
                IMU::instance()->setAccelRange(16);
                AccelRangeCounter++;
            }
            else if(AccelRangeCounter==4)
            {
                config.IMUAcellRange = 2;
                IMU::instance()->setAccelRange(2);
                AccelRangeCounter=1;
            }

            //Update configuration
            ConfigManager::instance()->setIMUConfig(config);

            //Save config
            if (ConfigManager::instance()->save_configuration())
            {
                printf("Configuration saved!\n");
            }
            else
            {
                printf("Error! Configuration not saved!\n");
            }
        }
        else
        {
            printf("Can't change Accelerometer range while logging\n");
        }
        
    }
    
    void WifiActions()
    {
        //Action of the wifi
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

//Activate the vibrating motor for the time asked
void VibrateMotor(int vibrate_time)
{
    IOExpander::instance().digitalWrite(EXT_PIN15_MOTOR_VIBRATE, HIGH);
    vTaskDelay(vibrate_time / portTICK_RATE_MS);
    IOExpander::instance().digitalWrite(EXT_PIN15_MOTOR_VIBRATE, LOW);
}

//app_main should have a "C" signature
extern "C"
{
    void app_main()
    {
        //This is required before using wifi
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);



        //This needs to be called first
        //install gpio isr service
        gpio_install_isr_service(0);

        //Set hardcoded timezone for now, TODO move somwhere else?
        setenv("TZ", "GEST+5EDT,M3.2.0/2,M11.1.0/2", 1);
        tzset();

        //Set time to JAN 1 2019
        struct tm tm;
        tm.tm_year = 2019 - 1900;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        tm.tm_hour = 1;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        time_t t = mktime(&tm);

        printf("Setting time: %s", asctime(&tm));
        struct timeval my_time;
        my_time.tv_sec = t;
        my_time.tv_usec = 0;
        settimeofday(&my_time, NULL);

        //esp_err_t ret=0;

        //SPI bus configuration
        SPIBus spibus;

        //I2C bus configuration
        I2CBus i2cbus;

        //I2C Ext bus configuration
        I2CBusExt i2cbusext;

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
        
        //PIN_NUM_ENABLE_PROGRAMMING = 
        //0 ---> CAN PROGRAM, 
        //1 ---> CANNOT PROGRAM WILL NOT RESET WHEN PLUGGING TO USB
        gpio_set_level((gpio_num_t)PIN_NUM_ENABLE_PROGRAMMING, 1);

        //Flashing Led Task
        TaskHandle_t ledBlinkHandle;
        xTaskCreate(&ledBlink, "Blinky", 2048, NULL, 1, &ledBlinkHandle);

        // SDCard must be started before configuration
        SDCard *sdcard = SDCard::instance();
        assert(sdcard);

        // Configuration will read configuration from sdcard...
        ConfigManager *configManager = ConfigManager::instance();
        assert(configManager);
        ConfigManager::instance()->print_configuration();

        //Mac Address
        uint8_t macaddress[6];
        uint64_t bit64macaddress[6];
        esp_read_mac(macaddress,ESP_MAC_WIFI_STA);
        for (int i=0;i<sizeof(macaddress);i++)
        {
            bit64macaddress[i]=macaddress[i];  
        }
        
        //Display
        Display *display = Display::instance();
        assert(display);
        display->begin();
        display->showSplashScreen(bit64macaddress);
        TickType_t splashTime = xTaskGetTickCount();

        Buttons *buttons = Buttons::instance();
        assert(buttons);
 
        Power *power = Power::instance();
        assert(power);

        GPS*  gps = GPS::instance();
        assert(gps);

        IMU *imu = IMU::instance();
        assert(imu);

        Barometer *baro = Barometer::instance();
        assert(baro);

        //Pulse should be started from menu?
        //Not yet working...
        //Pulse *pulse = Pulse::instance();
        //assert(pulse);

        //Bluetooth *ble = Bluetooth::instance();
        //assert(ble);

        //Prototype WiFi transfer Agent
        //WiFiTransfer *wifi = WiFiTransfer::instance();

        // HOMESCREEN
        Widget::Battery batteryWidget;
        batteryWidget.updateValue(5.0, 0.0, true);

        Widget::GPS gpsWidget;
        gpsWidget.setStatus(false);

        Widget::Log logWidget(Actions::IMUStartSD);
        logWidget.setStatus(false,true);

        Widget::SD sdWidget(Actions::ToggleSD);
        sdWidget.setStatus(false,true);

        Widget::Wifitransfer wifiWidget(Actions::WifiActions);

        Homescreen home;
        home.addWidget(&batteryWidget);
        home.addWidget(&gpsWidget);
        home.addWidget(&logWidget);
        home.addWidget(&sdWidget);
        home.addWidget(&wifiWidget);

        home.replaceSelection();

        // CONFIG SCREEN
        Widget::SampleRate sampleWidget(Actions::ChangeSampleRate);
        sampleWidget.setStatus(2);

        Widget::GyroRange gyroWidget(Actions::ChangeGyroRange);
        gyroWidget.setStatus(2);

        Widget::AccelRange accelWidget(Actions::ChangeAccelRange);
        accelWidget.setStatus(2);

        Widget::SDFreeSpace sdfreespaceWidget;
        sdfreespaceWidget.setStatus(16.0);

        Homescreen config;
        config.addWidget(&sampleWidget);
        config.addWidget(&gyroWidget);
        config.addWidget(&accelWidget);
        config.addWidget(&sdfreespaceWidget);

        config.replaceSelection();
        config.setVisible(false);

        // Show homescreen and disable programming
        vTaskDelayUntil(&splashTime, 4200 / portTICK_RATE_MS);
        //ioExpander.digitalWrite(EXT_PIN15_MOTOR_VIBRATE, HIGH);
        //vTaskDelay(800 / portTICK_RATE_MS);
        //ioExpander.digitalWrite(EXT_PIN15_MOTOR_VIBRATE, LOW);
        VibrateMotor(400);
        //gpio_set_level((gpio_num_t)PIN_NUM_ENABLE_PROGRAMMING, 0);
        home.setVisible(true);

        //Debug
        //Actions::IMUStartSD();
        //Do better...
        TickType_t lastBtn = xTaskGetTickCount();
        TickType_t lastRefresh = xTaskGetTickCount();
        TickType_t now;
        bool active = false;
        bool configscreen = false;
        bool _batteryLowMode = false;

        //Intialise counter  for message of widget
        Actions::SampleRateCounter = IMU::instance()->getSampleRate();
        Actions::GyroRangeCounter = IMU::instance()->getGyroRange();
        Actions::AccelRangeCounter = IMU::instance()->getAccelRange();


        while(1)
        {
            //printf("%f %f \n", voltage, current);

            // Sleep for 100ms
            vTaskDelay(100 / portTICK_RATE_MS);

            //Check battery state

            if(power->last_voltage()<3.2)
            {
                _batteryLowMode = true;
            }
            else if (power->last_voltage()>3.3)
            {
                _batteryLowMode = false;
            }

           // Check buttons

            while(buttons->getActionCtn() > 0) 
            {
                if (active && !_batteryLowMode) 
                {
                    if (home.getVisible())
                    {
                        home.action();
                    }

                    if (config.getVisible())
                    {
                        config.action();
                    }
                }
                buttons->decrementActionCtn();
                lastBtn = xTaskGetTickCount();
            }

            while(buttons->getPreviousCtn() > 0) 
            {
                if (active && !_batteryLowMode) 
                {
                    if (home.getVisible())
                    {
                        home.previous();
                    }

                    if (config.getVisible())
                    {
                        config.previous();
                    }
                }
                buttons->decrementPreviousCtn();
                lastBtn = xTaskGetTickCount();
            }

            while(buttons->getNextCtn() > 0) 
            {
                if (active && !_batteryLowMode) 
                {
                    if (home.getVisible())
                    {
                        home.next();
                    }

                    if (config.getVisible())
                    {
                        config.next();
                    }
                }
                buttons->decrementNextCtn();
                lastBtn = xTaskGetTickCount();
            }

            while(buttons->getBackCtn() > 0) 
            {
                if (active && !configscreen && !_batteryLowMode)
                {  
                    config.replaceSelection();
                    home.setVisible(false);
                    config.setVisible(true);
                    configscreen = true;

                    VibrateMotor(300);
                }

                else if (active && configscreen && !_batteryLowMode)
                {  
                    home.replaceSelection();
                    config.setVisible(false);
                    home.setVisible(true);
                    configscreen = false;

                    VibrateMotor(300);
                }
                buttons->decrementBackCtn();
                lastBtn = xTaskGetTickCount();
            }
    
            //Update widgets state
            batteryWidget.updateValue(power->last_voltage(), power->last_current(), power->last_charging());
            gpsWidget.setStatus(gps->getFix());
            logWidget.setStatus(Actions::loggingEnabled,SDCard::instance()->getSdCardPresent());
            sdWidget.setStatus(Actions::sdcardExternal,SDCard::instance()->getSdCardPresent());
            sampleWidget.setStatus(Actions::SampleRateCounter);
            gyroWidget.setStatus(Actions::GyroRangeCounter);
            accelWidget.setStatus(Actions::AccelRangeCounter);
            sdfreespaceWidget.setStatus(SDCard::instance()->getSDfreespace());

            //Check logging status
            if (Actions::loggingEnabled && !Actions::wasLogging)
            {
                home.startLog(-1);
                home.setLogID(SDCard::instance()->getlogID());
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

            // Update time and battery mode each second
            if (now-lastRefresh > 1000/portTICK_RATE_MS)
            {
                if (home.getVisible())
                {
                    home.updateBatteryMode(_batteryLowMode);
                    home.setVisible(true);
                    lastRefresh = now;
                }

                if (config.getVisible())
                {                   
                    config.updateBatteryMode(_batteryLowMode);
                    config.setVisible(true);
                    lastRefresh = now;
                }
            }
        } //while 
    }
}
