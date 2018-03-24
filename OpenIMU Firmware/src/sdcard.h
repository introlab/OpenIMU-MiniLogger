#ifndef SDCARD_H
#define SDCARD_H

#include <FS.h>
#include <Arduino.h>

#include "imu.h"

union imuDataSendable_t {
    imuData_t data;
    uint8_t bytes[sizeof(imuData_t)];
};

class SDCard
{
public:
    SDCard();
    virtual ~SDCard();

    void begin();

    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

    void toESP32();
    void toExternal();

    void startLog(QueueHandle_t queue);
    void stopLog();
};

#endif
