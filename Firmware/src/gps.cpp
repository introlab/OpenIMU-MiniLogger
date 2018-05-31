#include "gps.h"
#include "minmea.h"
#include <sys/time.h>

namespace
{
    HardwareSerial _gpsSerial(2);
    Adafruit_GPS _gps(&_gpsSerial);


    SemaphoreHandle_t flagMutex = xSemaphoreCreateMutex();
    bool timeIsSet = false;
    bool logToSerial = false;

    QueueHandle_t _loggingQueue = NULL;
    SemaphoreHandle_t _sdDataSemaphore = NULL;

    void gpsRead(void *pvParameters);
    void gpsToSerial();


    gpsData_ptr createDataPoint();
    void setTimeFromGPS(const struct minmea_date *date, const struct minmea_time *time_);
}

bool GPS::_hasBegun = false;

bool _validData = false;

GPS::GPS()
{

}

GPS::~GPS()
{

}

void GPS::begin()
{
    if(!_hasBegun) {


      Serial.println("Starting GPS.");

        _gps.begin(9600);


        //_gps.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
        //_gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
        //_gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
        //_gps.sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
        //_gps.sendCommand(PGCMD_ANTENNA);
        //_gps.sendCommand(PGCMD_NOANTENNA);
        //_gpsSerial.println(PMTK_Q_RELEASE);


        //flagMutex = xSemaphoreCreateMutex();
        xTaskCreatePinnedToCore(&gpsRead, "GPS Read", 2048, NULL, 5, NULL, 1);
    }
}

void GPS::startSerialLogging()
{
    xSemaphoreTake(flagMutex, portMAX_DELAY);
    logToSerial = true;
    xSemaphoreGive(flagMutex);
}

void GPS::stopSerialLogging()
{
    xSemaphoreTake(flagMutex, portMAX_DELAY);
    logToSerial = false;
    xSemaphoreGive(flagMutex);
}

void GPS::startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore)
{
    xSemaphoreTake(flagMutex, portMAX_DELAY);
    _loggingQueue = queue;
    _sdDataSemaphore = semaphore;
    xSemaphoreGive(flagMutex);
}

void GPS::stopQueueLogging()
{
    xSemaphoreTake(flagMutex, portMAX_DELAY);
    _loggingQueue = NULL;
    _sdDataSemaphore = NULL;
    xSemaphoreGive(flagMutex);
}

// void GPS::testaffich()
// {
//    //gpsRead();
//    gpsToSerial();
// }

namespace
{
    void gpsRead(void *pvParameters)
    {
        int i=0;
        bool init=true;

        bool serialEnabled, queueEnabled;
        TickType_t lastTick = xTaskGetTickCount();

        while(1) {


            while(_gpsSerial.available()) {
                 

                //Read one byte
                uint8_t c = _gps.read();
                //Print for debug
                //Serial.write(c);

                if(_gps.newNMEAreceived()) {
                    //_gps.parse(_gps.lastNMEA());
                    char* ptr = _gps.lastNMEA();
                    char* ptrcopy ;
                    //There is a bug, sentense starts with \n and the $...
                    ptr++;
                    
                    // i++;
                    // if(i==10){
                        
                    // //Serial.printf("len %i, %s\n", strlen(_gps.lastNMEA()), _gps.lastNMEA());
                    // // Serial.printf("%c%c%c%c%c%c\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
                    // Serial.printf("%s\n", ptr);
                    // i=0;
                    // }


                    int id = minmea_sentence_id(ptr, true);
                    //Serial.printf("Sentense id: %i \n", id);
                    // Serial.print(id);
                    // Serial.print("\n"); //RMC
                    switch(id)
                    {
                        
                        case MINMEA_SENTENCE_RMC:
                          struct minmea_sentence_rmc rmc;
                        //    Serial.printf("%s\n", ptr); 
                            
                            if (init)
                            { // At the start of the device, get time from gts rtc using RMC message
                            init=false;

                            struct tm tm;
                            
                            int k=0;
                            while(*ptr){ // Structure of the message known 
                             char type = *ptr++;
                             if  (type == ',') break;
                             } // Extract time
                            tm.tm_hour = ( ptr[0] - 48 ) * 10  +  ( ptr[1] - 48 );
                            tm.tm_min = ( ptr[2] - 48 ) * 10  +  ( ptr[3] - 48 );
                            tm.tm_sec = ( ptr[4] - 48 ) * 10  +  ( ptr[5] - 48 );

                            while(*ptr){
                               char type = *ptr++;
                               if  (type == ',') k++;
                               if  (k==8) break;
                            } // Extract date
                            tm.tm_mday = ( ptr[0] - 48 ) * 10  +  ( ptr[1] - 48 );
                            tm.tm_mon = ( ptr[2] - 48 ) * 10  +  ( ptr[3] - 48 ) - 1; // month is an offset to january [0-11]                         
                            tm.tm_year = ( ptr[4] - 48 ) * 10  +  ( ptr[5] - 48 ) + 100; // years since 1900
                            
                            struct timeval tvl;
                            tvl.tv_sec =  mktime(&tm) - (3600 * 4); // Change value according to your time zone 
                            tvl.tv_usec = 0;
                            settimeofday(&tvl, NULL);
                            
                            }
                            else// Normal Use of RMC messages
                            { 
                            if (minmea_parse_rmc(&rmc, ptr))
                            {

                                   if (rmc.valid)
                                {

                                   _validData = true;
                                /*
                                struct minmea_float latitude;
                                struct minmea_float longitude;
                                struct minmea_float speed;
                                struct minmea_float course;
                                struct minmea_float variation;
                                */
                                //Serial.printf("Date: %2i/%2i/%4i\n", rmc.date.day, rmc.date.month, rmc.date.year);
                                //Serial.printf("Time: %2i:%2i:%2i\n", rmc.time.hours, rmc.time.minutes, rmc.time.seconds);
                                setTimeFromGPS(&rmc.date, &rmc.time);

                                float latitude = minmea_tocoord(&rmc.latitude);
                                float longitude = minmea_tocoord(&rmc.longitude);
                                //Serial.printf("Latitude %f, Longitude %f \n", latitude, longitude);

                                }
                                else
                                {
                                     _validData = false;
                                }

                            }

                            }

            
                          

                        break;

                        case MINMEA_SENTENCE_GGA:
                        struct minmea_sentence_gga gga;
                        if (minmea_parse_gga(&gga, ptr))
                        {
                          /*
                          struct minmea_sentence_gga {
                              struct minmea_time time;
                              struct minmea_float latitude;
                              struct minmea_float longitude;
                              int fix_quality;
                              int satellites_tracked;
                              struct minmea_float hdop;
                              struct minmea_float altitude; char altitude_units;
                              struct minmea_float height; char height_units;
                              int dgps_age;
                          };
                          */
                          int sat = gga.satellites_tracked;
                          float latitude = minmea_tocoord(&gga.latitude);
                          float longitude = minmea_tocoord(&gga.longitude);
                          float altitude = minmea_tofloat(&gga.altitude);
                          //Serial.printf("Sats: %i, Latitude %f, Longitude %f \n", sat, latitude , longitude );
                          _gps.latitude = latitude;
                          _gps.fix = true;
                          _gps.longitude = longitude;
                          _gps.altitude = altitude;

                          // Log to queue


                          xSemaphoreTake(flagMutex, portMAX_DELAY);
                          if(_loggingQueue != NULL) {
                              gpsData_ptr measure = createDataPoint();
                              if(xQueueSend(_loggingQueue, (void *) &measure, 0) != pdTRUE) {
                                  Serial.println("Queue is full! Dropping GPS measure");
                                  free(measure);
                              }
                              else {
                                  //Will wake up writing task
                                  xSemaphoreGive(_sdDataSemaphore);
                              }
                          }
                          xSemaphoreGive(flagMutex);


                        }
                        break;

                        case MINMEA_SENTENCE_VTG:
                        //TODO
                        break;

                        case MINMEA_SENTENCE_ZDA:
                        
                          //Time AND Date
                          struct minmea_sentence_zda zda;
                          if (minmea_parse_zda(&zda, ptr))
                          {
                            /*
                            struct minmea_sentence_zda {
                                struct minmea_time time;
                                struct minmea_date date;
                                int hour_offset;
                                int minute_offset;
                            };
                            */
                            Serial.println("should set time");
                            setTimeFromGPS(&zda.date, &zda.time);
                          }
                        break;

                        case MINMEA_SENTENCE_GSA:
                        //TODO
                        break;

                        case MINMEA_SENTENCE_GLL:
                        //TODO
                        break;

                        case MINMEA_SENTENCE_GSV:
                        //TODO
                        break;

                        default:
                            Serial.printf("Unhandled id: %i\n", id);
                        break;

                    }
                }

            }


            vTaskDelay(10/portTICK_RATE_MS);
        }
    }

    gpsData_ptr createDataPoint()
    {
        //TODO FIX THIS PART...
        gpsData_ptr data;

        data = (gpsData_ptr) malloc(sizeof(gpsData_t));

        data->fix = _gps.fix;
        data->longitude = _gps.longitude;
        data->latitude = _gps.latitude;
        data->altitude = _gps.altitude;

        return data;
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

    void setTimeFromGPS(const struct minmea_date *date, const struct minmea_time *time_)
    { // Automatic Time Update to fix (not the good hours)
        struct timeval timeval;
        minmea_gettime(&timeval, date, time_);

        // struct timezone timezone;
        // timezone.tz_minuteswest = -5 * 60;
        // timezone.tz_dsttime = 0; //DST_CAN;
       
        settimeofday(&timeval, NULL);
        //Serial.println("Got time from GPS");

    }
}

bool GPS::getFlagvalidData()
{
    return _validData;
}