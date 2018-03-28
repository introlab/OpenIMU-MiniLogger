#include "gps.h"

namespace
{
    HardwareSerial _gpsSerial(2);
    Adafruit_GPS _gps(&_gpsSerial);
    GPSMutex _gpsMutex;
    bool timeIsSet = false;

    void gpsRead(void *pvParameters);
    void gpsToSerial();

    time_t computeTime();
    void setTime();
}

bool GPS::_hasBegun = false;

GPS::GPS()
{

}

GPS::~GPS()
{

}

void GPS::begin()
{
    if(!_hasBegun) {
        _gpsMutex.acquire();
        _gps.begin(9600);

        _gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
        _gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
        _gps.sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
        _gps.sendCommand(PGCMD_ANTENNA);
        _gpsMutex.release();

        _gpsSerial.println(PMTK_Q_RELEASE);

        xTaskCreate(&gpsRead, "GPS Read", 2048, NULL, 5, NULL);
    }
}

namespace
{
    void gpsRead(void *pvParameters)
    {
        while(1) {
            while(_gpsSerial.available()) {
                _gpsMutex.acquire();
                _gps.read();

                if(_gps.newNMEAreceived()) {
                    _gps.parse(_gps.lastNMEA());
                    if(_gps.fix && !timeIsSet) {
                        setTime();
                        timeIsSet = true;
                    }
                }
                _gpsMutex.release();
            }
            vTaskDelay(10/portTICK_RATE_MS);
        }
    }

    void gpsToSerial()
    {
        Serial.print("\nTime: ");
        Serial.print(_gps.hour, DEC); Serial.print(':');
        Serial.print(_gps.minute, DEC); Serial.print(':');
        Serial.print(_gps.seconds, DEC); Serial.print('.');
        Serial.println(_gps.milliseconds);
        Serial.print("Date: ");
        Serial.print(_gps.day, DEC); Serial.print('/');
        Serial.print(_gps.month, DEC); Serial.print("/20");
        Serial.println(_gps.year, DEC);
        Serial.print("Fix: "); Serial.print((int)_gps.fix);
        Serial.print(" quality: "); Serial.println((int)_gps.fixquality);

        if (_gps.fix) {
          Serial.print("Location: ");
          Serial.print(_gps.latitude, 4); Serial.print(_gps.lat);
          Serial.print(", ");
          Serial.print(_gps.longitude, 4); Serial.println(_gps.lon);
          Serial.print("Speed (knots): "); Serial.println(_gps.speed);
          Serial.print("Angle: "); Serial.println(_gps.angle);
          Serial.print("Altitude: "); Serial.println(_gps.altitude);
          Serial.print("Satellites: "); Serial.println((int)_gps.satellites);
        }
    }

    time_t computeTime()
    {
        struct tm currentTime;

        currentTime.tm_sec = _gps.seconds;
        currentTime.tm_min = _gps.minute;
        currentTime.tm_hour = _gps.hour;
        currentTime.tm_mday = _gps.day;
        currentTime.tm_mon = _gps.month - 1;
        currentTime.tm_year = _gps.year + 100;

        return mktime(&currentTime);
    }

    void setTime()
    {
        struct timeval timeval;
        timeval.tv_sec = computeTime();
        timeval.tv_usec = 0;

        struct timezone timezone;
        timezone.tz_minuteswest = 0;
        timezone.tz_dsttime = 0;

        settimeofday(&timeval, &timezone);
        Serial.println("Got time from GPS");
    }
}
