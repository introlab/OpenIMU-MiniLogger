#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include <ctime>
#include <sys/time.h>

#include "gpsmutex.h"

typedef struct {
    float latitude;
    float longitude;
    float altitude;
} gpsData_t;

class GPS
{
public:
    GPS();
    virtual ~GPS();

    static void begin();

    //static void startSerialLogging();
    //static void stopSerialLogging();

    //static void startQueueLogging();
    //static void stopQueueLogging();

private:
    static bool _hasBegun;
};

#endif
