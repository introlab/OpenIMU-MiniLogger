#include <Arduino.h>

#define FIRSTBOOT


#include <ioexpander.h>
#include <SPI.h>
#include "defines.h"
#include "Wire.h"
#include "actions.h"
#include "soc/rtc.h"

IOExpander ioExpander;

void ledBlink(void *pvParameters)
{
  while(1)
  {
    ioExpander.digitalWrite(EXT_PIN01_LED, LOW);

    delay(500);

    ioExpander.digitalWrite(EXT_PIN01_LED, HIGH);

    delay(500);
  }
}

#ifndef FIRSTBOOT
#include "imu.h"
#include "sdcard.h"
#include "buttons.h"
#include "menu.h"
#include "display.h"
#include "barometer.h"
#include "gps.h"
#include "adc.h"
#include <Esp.h>

IMU imu;
SDCard sdCard;
Buttons buttons;
Menu menu;
Display display;
Barometer baro;
GPS gps;
ADC adc;
EspClass esp;

uint64_t mac_adress;
bool log_flag = false;
bool SD_USB_flag = false;

QueueHandle_t imuLoggingQueue = NULL;
QueueHandle_t gpsLoggingQueue = NULL;
QueueHandle_t powerLoggingQueue = NULL;
QueueHandle_t baroLoggingQueue = NULL;
SemaphoreHandle_t sdDataReadySemaphore = NULL;


void printCurrentTime();
#endif

TaskHandle_t ledBlinkHandle = NULL;


void setup_gpio()
{
  // Setup I2C PINS, SDA, SCL, Clock speed
  //Wire.begin(23, 25, 400000);
  //EXP Chip Select
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);

  //SCLK
  pinMode(19,OUTPUT);
  //MOSI
  pinMode(18,OUTPUT);
  //MISO
  pinMode(39, INPUT);
  //nINT
  pinMode(36, INPUT_PULLUP);

  //SCK, MISO, MOSI no SS pin
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.begin(19, 39, 18);


  //pinMode(23, INPUT);
  //pinMode(25, INPUT);



  ioExpander.begin();


  //ALIVE -->HIGH, power will stay on
  ioExpander.pinMode(EXT_PIN12_KEEP_ALIVE, OUTPUT);
  ioExpander.digitalWrite(EXT_PIN12_KEEP_ALIVE, HIGH);

  //LED
  ioExpander.pinMode(EXT_PIN01_LED, OUTPUT);
  ioExpander.digitalWrite(EXT_PIN01_LED, HIGH);

}

void setup() {

    // This must be the first thing we do.
    setup_gpio();

    //On Arduino Core
    xTaskCreatePinnedToCore(&ledBlink, "Blinky", 2048, NULL, 1, &ledBlinkHandle,1);




#ifndef FIRSTBOOT
    // Start serial
    Serial.begin(115200);
    Serial.println("Starting...");
    //Serial.println(String(rtc_clk_cpu_freq_get()));
    Serial.println("----");


    //Serial.println("Setting clock to 80MHz");
    /*
    typedef enum {
        RTC_CPU_FREQ_XTAL = 0,      //!< Main XTAL frequency
        RTC_CPU_FREQ_80M = 1,       //!< 80 MHz
        RTC_CPU_FREQ_160M = 2,      //!< 160 MHz
        RTC_CPU_FREQ_240M = 3,      //!< 240 MHz
        RTC_CPU_FREQ_2M = 4,        //!< 2 MHz
    } rtc_cpu_freq_t;
    */
    //Not supported, the CPU runs at 240MHz right now...
    //rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
    Serial.println(String(rtc_clk_cpu_freq_get()));
    Serial.println("----");


    // Start display
    display.begin();
    //display.showMenu(&menu);
    display.clear();
    Serial.println("Display Ready");

    //Start Buttons
    buttons.begin();
    Serial.println("Buttons Ready");

    // Initialize SD-card
    sdCard.begin();
    Serial.println("SD Card ready");

    //Wire.setClock(100000);
    //Wire.setTimeOut(200);


    Wire.begin(23, 25, 100000);
    //Wire.flush();
    //Wire.endTransmission();

    // Start IMU
    imu.begin();
    Serial.println("IMU Ready");

    // Start Barometer
    baro.begin();
    Serial.println("Barometer Ready");

    // Start GPS
    gps.begin();
    Serial.println("GPS Ready");

    // Start ADC
    adc.begin();
    Serial.println("ADC Ready");

    //All ready!
    //Make sure button counts are reset
    buttons.reset();

    Serial.println("System ready");

    mac_adress = esp.getEfuseMac();
    Serial.printf("Mac : %" PRIu64 "\n", mac_adress);


#endif

}

void loop() {

#ifndef FIRSTBOOT


    //Display menu
    display.updateMenu(&menu, log_flag);

    int change_counter = 0;

    //Actions::IMUStartSD();

    while(1)
    {
        bool changed = false;

        // Button Shutdown, keep pushing button until led stop blinking
        if(buttons.getShutDown())
        {
            Serial.println("Shutting down.");
            vTaskDelete( ledBlinkHandle );
            Actions::IMUStopSD();
            //sdCard.toExternal();
            display.showSplashScreen(mac_adress);
            Serial.println("Bye!");
            ioExpander.digitalWrite(EXT_PIN12_KEEP_ALIVE, LOW);
        }


        while(buttons.getActionCtn() > 0) {
            menu.action();
            buttons.decrementActionCtn();
            changed = true;
        }

        while(buttons.getPreviousCtn() > 0) {
            menu.previous();
            buttons.decrementPreviousCtn();
            changed = true;
        }

        while(buttons.getNextCtn() > 0) {
            menu.next();
            buttons.decrementNextCtn();
            changed = true;
        }

        while(buttons.getBackCtn() > 0) {
            display.displayVoltage(adc.getVoltage(), adc.getCurrent(),gps.getFlagvalidData(), log_flag, SD_USB_flag);
            buttons.decrementBackCtn();
        }



        if(changed) {
            //Serial.print("Registered press. ");
            display.updateMenu(&menu,log_flag);
            //Serial.println("Display menu.");
            change_counter = 0;
        }
        else {
          change_counter++;

          // Every 5 seconds verify if no activity, then paint state
          if (change_counter > 50)
          {
              //Serial.println("Display voltage");
              display.displayVoltage(adc.getVoltage(), adc.getCurrent(),gps.getFlagvalidData(), log_flag, SD_USB_flag);

              change_counter = 0;
          }

        }

        delay(100);
        //printCurrentTime();
        //Serial.printf("Batt: %f V, %f A\n", adc.getVoltage(), adc.getCurrent());

  }//while

  #endif
}

#ifndef FIRSTBOOT
void printCurrentTime()
{
    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = gmtime(&now);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", timeinfo);

    Serial.print("Current time ");
    Serial.println(strftime_buf);
}

namespace Actions
{
    // Software Shutdown by the menu
    void Shutdown()
    {
        Serial.println("Shutting down.");
        vTaskDelete( ledBlinkHandle );
        Actions::IMUStopSD();
        display.showSplashScreen(mac_adress);
        Serial.println("Bye!");
        ioExpander.digitalWrite(EXT_PIN12_KEEP_ALIVE, LOW);
    }


    void SDToESP32()
    {
        if (SD_USB_flag == true)
        {
        sdCard.toESP32();
        SD_USB_flag = false;
        }
    }

    void SDToExternal()
    {
        if (SD_USB_flag == false)
        {
        sdCard.toExternal();
        SD_USB_flag = true;
        }
    }

    //Same function to start and stop logging to avoid double start
    void IMUStartSD()     {
         if(imuLoggingQueue != NULL
            && gpsLoggingQueue != NULL
            && powerLoggingQueue != NULL
            && baroLoggingQueue != NULL)    // stop case
            {

            imu.stopQueueLogging();
            gps.stopQueueLogging();
            adc.stopQueueLogging();
            baro.stopQueueLogging();

            sdCard.stopLog();
            sdCard.setIMUQueue(NULL);
            sdCard.setGPSQueue(NULL);
            sdCard.setPowerQueue(NULL);
            sdCard.setBarometerQueue(NULL);
            sdCard.setDataReadySemaphore(NULL);

            vSemaphoreDelete(sdDataReadySemaphore);
            vQueueDelete(imuLoggingQueue);
            vQueueDelete(gpsLoggingQueue);
            vQueueDelete(powerLoggingQueue);
            vQueueDelete(baroLoggingQueue);

            sdDataReadySemaphore = NULL;
            imuLoggingQueue = NULL;
            gpsLoggingQueue = NULL;
            powerLoggingQueue = NULL;
            baroLoggingQueue = NULL;
            log_flag = false;
            }
            else        // start case

            {

            if (SD_USB_flag == true)
            {
            sdCard.toESP32();
            SD_USB_flag = false;
            }
            imuLoggingQueue = xQueueCreate(20, sizeof(imuData_ptr));
            gpsLoggingQueue = xQueueCreate(10, sizeof(gpsData_t));
            powerLoggingQueue = xQueueCreate(10, sizeof(powerData_ptr));
            baroLoggingQueue = xQueueCreate(10, sizeof(baroData_ptr));
            sdDataReadySemaphore = xSemaphoreCreateCounting(128, 0);

            sdCard.setIMUQueue(imuLoggingQueue);
            sdCard.setGPSQueue(gpsLoggingQueue);
            sdCard.setPowerQueue(powerLoggingQueue);
            sdCard.setBarometerQueue(baroLoggingQueue);
            sdCard.setDataReadySemaphore(sdDataReadySemaphore);
            sdCard.startLog();

            //TODO Add other sensors...
            gps.startQueueLogging(gpsLoggingQueue, sdDataReadySemaphore);
            imu.startQueueLogging(imuLoggingQueue, sdDataReadySemaphore);
            adc.startQueueLogging(powerLoggingQueue, sdDataReadySemaphore);
            baro.startQueueLogging(baroLoggingQueue, sdDataReadySemaphore);

            log_flag = true;
            }

    }

    void IMUStopSD()    // used to be sure to stop log when device shutdown
    {
        if(imuLoggingQueue != NULL
            && gpsLoggingQueue != NULL
            && powerLoggingQueue != NULL
            && baroLoggingQueue != NULL) {

            imu.stopQueueLogging();
            gps.stopQueueLogging();
            adc.stopQueueLogging();
            baro.stopQueueLogging();

            sdCard.stopLog();
            sdCard.setIMUQueue(NULL);
            sdCard.setGPSQueue(NULL);
            sdCard.setPowerQueue(NULL);
            sdCard.setBarometerQueue(NULL);
            sdCard.setDataReadySemaphore(NULL);

            vSemaphoreDelete(sdDataReadySemaphore);
            vQueueDelete(imuLoggingQueue);
            vQueueDelete(gpsLoggingQueue);
            vQueueDelete(powerLoggingQueue);
            vQueueDelete(baroLoggingQueue);

            sdDataReadySemaphore = NULL;
            imuLoggingQueue = NULL;
            gpsLoggingQueue = NULL;
            powerLoggingQueue = NULL;
            baroLoggingQueue = NULL;

            log_flag = false;
        }
    }
}
#endif
