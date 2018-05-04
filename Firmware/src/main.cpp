#include <Arduino.h>

#if 0
#include "display.h"
#include "menu.h"
#include "sdcard.h"
#include "buttons.h"
#include "imu.h"
#include "gps.h"
Display display;
Menu menu;
SDCard sdCard;
Buttons buttons;
IMU imu;
GPS gps;
#endif

#include "MCP23S17.h"
#include <SPI.h>




//Address = 0, CS=5
MCP mcp23s17(0,5);

QueueHandle_t imuLoggingQueue = NULL;
QueueHandle_t gpsLoggingQueue = NULL;
SemaphoreHandle_t sdDataReadySemaphore = NULL;

void printCurrentTime();

void setup_gpio()
{


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
  mcp23s17.begin();

  //ALIVE -->HIGH, power will stay on
  mcp23s17.pinMode(12 + 1, OUTPUT);
  mcp23s17.digitalWrite(12 + 1, HIGH);

  mcp23s17.pinMode(2, OUTPUT);
  mcp23s17.digitalWrite(2, HIGH);

}

void setup() {

    setup_gpio();

    // Start serial
    Serial.begin(115200);

    Serial.println("Setting GPIO for SPI");
#if 0
    // Start display
    Serial.println("Initializing display...");
    delay(1000);

    // Show menu and start reading buttons
    display.begin();
    display.showMenu(&menu);
    buttons.begin();

    Serial.println("Display Ready");

    // Initialize SD-card
    sdCard.begin();

    // Start IMU
    imu.begin();

    // Start GPS
    gps.begin();
#endif
    Serial.println("System ready");
}

void loop() {

  while(1)
  {
    Serial.println("Loop");
    mcp23s17.digitalWrite(2, LOW);
    delay(500);

    mcp23s17.digitalWrite(2, HIGH);
    delay(500);
 }
#if 0

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

#endif
}

#if 0
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
        imu.startSerialLogging();
        gps.startSerialLogging();
    }

    void IMUStopSerial()
    {
        imu.stopSerialLogging();
        gps.stopSerialLogging();
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

        gps.startQueueLogging(gpsLoggingQueue, sdDataReadySemaphore);
        imu.startQueueLogging(imuLoggingQueue, sdDataReadySemaphore);
    }

    void IMUStopSD()
    {
        if(imuLoggingQueue != NULL && gpsLoggingQueue != NULL) {
            imu.stopQueueLogging();
            gps.stopQueueLogging();

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
#endif
