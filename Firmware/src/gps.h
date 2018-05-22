#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include <ctime>
#include <sys/time.h>

typedef struct {
    bool fix;
    float latitude;
    float longitude;
    float altitude;
} gpsData_t;

//Command DEFINITIONS



class GPS
{
public:
    GPS();
    virtual ~GPS();

    static void begin();

    static void startSerialLogging();
    static void stopSerialLogging();

    static void startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore);
    static void stopQueueLogging();

private:
    static bool _hasBegun;
};

#endif
