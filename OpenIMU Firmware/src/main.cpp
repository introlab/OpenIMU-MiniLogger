#include <Arduino.h>

#include "SparkFunMPU9250-DMP.h"

#include "display.h"
#include "menu.h"
#include "sdcard.h"
#include "buttons.h"
#include "imu.h"

Display display;
Menu menu;
SDCard sdCard;
Buttons buttons;
IMU imu;
QueueHandle_t imuLoggingQueue = NULL;


void setup() {

    // Start serial
    Serial.begin(115200);
    delay(3000);

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

    Serial.println("System ready");
}

void loop() {
    static unsigned long counter = 0;

    while(buttons.getActionCtn() > 0) {
        menu.action();
        buttons.decrementActionCtn();
    }

    while(buttons.getPreviousCtn() > 0) {
        menu.previous();
        buttons.decrementPreviousCtn();
    }

    while(buttons.getNextCtn() > 0) {
        menu.next();
        buttons.decrementNextCtn();
    }

    display.updateMenu(&menu);
    Serial.print("Refreshed display ");
    Serial.println(counter++);
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
    }

    void IMUStopSerial()
    {
        imu.stopSerialLogging();
    }

    void IMUStartSD()
    {
        imuLoggingQueue = xQueueCreate(20, sizeof(imuData_ptr));
        sdCard.startLog(imuLoggingQueue);
        imu.startQueueLogging(imuLoggingQueue);
    }

    void IMUStopSD()
    {
        imu.stopQueueLogging();
        sdCard.stopLog();
        vQueueDelete(imuLoggingQueue);
    }
}
