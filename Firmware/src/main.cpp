#include <Arduino.h>

#if 0
#include "display.h"
#include "gps.h"
Display display;
GPS gps;
#endif

#include "MCP23S17.h"
#include <SPI.h>
#include "imu.h"
#include "Wire.h"
#include "sdcard.h"
#include "buttons.h"
#include "defines.h"
#include "menu.h"
#include "display.h"
#include "Adafruit_MPL115A2.h"
#include <ioexpander.h>

//Address = 0, CS=5
//MCP mcp23s17(0,5);
IOExpander ioExpander;
IMU imu;
SDCard sdCard;
Buttons buttons;
Menu menu;
Display display;
//For Testing
Adafruit_MPL115A2 baro;


QueueHandle_t imuLoggingQueue = NULL;
QueueHandle_t gpsLoggingQueue = NULL;
SemaphoreHandle_t sdDataReadySemaphore = NULL;

void printCurrentTime();

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
  SPI.begin(19, 39, 18);

  ioExpander.begin();


  //ALIVE -->HIGH, power will stay on
  ioExpander.pinMode(EXT_PIN12_KEEP_ALIVE, OUTPUT);
  ioExpander.digitalWrite(EXT_PIN12_KEEP_ALIVE, HIGH);

  //LED
  ioExpander.pinMode(EXT_PIN01_LED, OUTPUT);
  ioExpander.digitalWrite(EXT_PIN01_LED, HIGH);

  /*
  mcp23s17.begin();

  //ALIVE -->HIGH, power will stay on
  mcp23s17.pinMode(EXT_PIN12_KEEP_ALIVE, OUTPUT);
  mcp23s17.digitalWrite(EXT_PIN12_KEEP_ALIVE, HIGH);

  //LED
  mcp23s17.pinMode(EXT_PIN01_LED, OUTPUT);
  mcp23s17.digitalWrite(EXT_PIN01_LED, HIGH);
  */

}

void setup() {

    // This must be the first thing we do.
    setup_gpio();

    // Start serial
    Serial.begin(115200);

    // Start display
    //Serial.println("Initializing display...");
    //delay(1000);


    buttons.begin();

    Serial.println("Display Ready");

    // Initialize SD-card
    sdCard.begin();

    // Start IMU
    imu.begin();

    //TODO for testing only
    baro.begin();

    // Show menu and start reading buttons
    display.begin();
    display.showMenu(&menu);



    // Start GPS
    //gps.begin();

    Serial.println("System ready");
}

void loop() {


    Serial.println("Loop");
    ioExpander.digitalWrite(EXT_PIN01_LED, LOW);

    delay(500);

    ioExpander.digitalWrite(EXT_PIN01_LED, HIGH);

    delay(500);

    //Testing only
    float temp = baro.getTemperature();
    float pressure = baro.getPressure();
    printf("temp: %f, pressure: %f\n", temp, pressure);


    bool changed = false;

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


    if(changed) {
        Serial.print("Registered press. ");
        display.updateMenu(&menu);
        Serial.println("Refreshed display.");
    }

}


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
    void SDToESP32()
    {
        sdCard.toESP32();
    }

    void SDToExternal()
    {
        sdCard.toExternal();
    }

    void IMUStartSerial()
    {
        //imu.startSerialLogging();
        //TODO ADD GPS
        //gps.startSerialLogging();
    }

    void IMUStopSerial()
    {
        //imu.stopSerialLogging();
        //TODO ADD GPS
        //gps.stopSerialLogging();
    }

    void IMUStartSD()
    {
        imuLoggingQueue = xQueueCreate(20, sizeof(imuData_ptr));
        gpsLoggingQueue = xQueueCreate(10, sizeof(gpsData_t));
        sdDataReadySemaphore = xSemaphoreCreateCounting(128, 0);

        sdCard.setIMUQueue(imuLoggingQueue);
        sdCard.setGPSQueue(gpsLoggingQueue);
        sdCard.setDataReadySemaphore(sdDataReadySemaphore);
        sdCard.startLog();

        //TODO ADD GPS
        //gps.startQueueLogging(gpsLoggingQueue, sdDataReadySemaphore);
        imu.startQueueLogging(imuLoggingQueue, sdDataReadySemaphore);
    }

    void IMUStopSD()
    {
        if(imuLoggingQueue != NULL && gpsLoggingQueue != NULL) {
            imu.stopQueueLogging();
            //TODO Add GPS
            //gps.stopQueueLogging();

            sdCard.stopLog();
            sdCard.setIMUQueue(NULL);
            sdCard.setGPSQueue(NULL);
            sdCard.setDataReadySemaphore(NULL);

            vSemaphoreDelete(sdDataReadySemaphore);
            vQueueDelete(imuLoggingQueue);
            vQueueDelete(gpsLoggingQueue);

            sdDataReadySemaphore = NULL;
            imuLoggingQueue = NULL;
            gpsLoggingQueue = NULL;
        }
    }
}
