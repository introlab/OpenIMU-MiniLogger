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
#include "MPL115A2.h"


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
        xTaskCreate(&ledBlink, "Blinky", 2048, NULL, 1, &ledBlinkHandle);

        SDCard *sdcard = SDCard::instance();

        IMU *imu = IMU::instance();

        Power *power = Power::instance();

        Display *display = Display::instance();

        Menu menu;


        display->begin();
        display->clear();
        //display.showSplashScreen(0);

        //Debug
        SDCard::instance()->startLog();


        MPL115A2 test(I2C_NUM_1);
        printf("Temp: %f, Pressure %f\n", test.getTemperature(), test.getPressure());

        //Do better...
        while(1)
        {

            //imu->readSensor();

            //Buttons tests
            printf("B0: %i\n", ioExpander.digitalRead(EXT_PIN11_BUTTON0));
            printf("B1: %i\n", ioExpander.digitalRead(EXT_PIN06_BUTTON1));
            printf("B2: %i\n", ioExpander.digitalRead(EXT_PIN08_BUTTON2));
            printf("B3: %i\n", ioExpander.digitalRead(EXT_PIN09_BUTTON3));

            //ADC tests
            printf("Batt: %4.4f Current: %4.4f\n", power->read_voltage(), power->read_current());

            display->displayVoltage(power->read_voltage(), power->read_current(), false, false, false);
            //display.updateMenu(&menu, false);
            vTaskDelay(100 / portTICK_RATE_MS);
        }
    }
}
