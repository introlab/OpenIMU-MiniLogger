#include "gps.h"
#include <string.h>
#include <sys/time.h>
#include <ctime>
#include "sdcard.h"

namespace
{
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

    //Called from task
    void process_sentence(const char* sentence)
    {
        int id = minmea_sentence_id(sentence, true);
        switch(id)
        {
            case MINMEA_INVALID:
                printf("INVALID\n");
            break;

            case MINMEA_UNKNOWN:
                printf("MINMEA_UNKNOWN\n");
            break;

            case MINMEA_SENTENCE_RMC:
                //printf("MINMEA_SENTENCE_RMC\n");
                struct minmea_sentence_rmc rmc;
                if (minmea_parse_rmc(&rmc, sentence))
                {
                    if (rmc.valid)
                    {
                        /*
                        struct minmea_float latitude;
                        struct minmea_float longitude;
                        struct minmea_float speed;
                        struct minmea_float course;
                        struct minmea_float variation;
                        */
                    
                        setTimeFromGPS(&rmc.date, &rmc.time);

                        float latitude = minmea_tocoord(&rmc.latitude);
                        float longitude = minmea_tocoord(&rmc.longitude);
                        //printf("RMC latitude: %f, longitude: %f \n", latitude, longitude);

                    }
                }
            break;

            case MINMEA_SENTENCE_GGA:
                //printf("MINMEA_SENTENCE_GGA\n");
                struct minmea_sentence_gga gga;
                if (minmea_parse_gga(&gga, sentence))
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
                    //printf("GGA sat: %i, latitude: %f, longitude: %f, altitude: %f\n", sat, latitude, longitude, altitude);

                    gpsDataPtr_t data = (gpsDataPtr_t) malloc(sizeof(gpsData_t));
                    //Fill data
                    data->altitude = altitude;
                    data->fix = true;
                    data->latitude = latitude;
                    data->longitude = longitude;

                    //Send to logging thread
                    if (!SDCard::instance()->enqueue(data))
                        free(data);

                }
            break;

            case MINMEA_SENTENCE_GSA:
                //TODO
                //printf("MINMEA_SENTENCE_GSA\n");
            break;

            case MINMEA_SENTENCE_GLL:
                //TODO
                //printf("MINMEA_SENTENCE_GLL\n");
            break;

            case MINMEA_SENTENCE_GST:
                //TODO
                //printf("MINMEA_SENTENCE_GST\n");
            break;

            case MINMEA_SENTENCE_GSV:
                //TODO
                //printf("MINMEA_SENTENCE_GSV\n");
            break;

            case MINMEA_SENTENCE_VTG:
                //TODO
                //printf("MINMEA_SENTENCE_VTG\n");
            break;

            case MINMEA_SENTENCE_ZDA:
                //printf("MINMEA_SENTENCE_ZDA\n");
                //Time AND Date
                struct minmea_sentence_zda zda;
                if (minmea_parse_zda(&zda, sentence))
                {
                    /*
                    struct minmea_sentence_zda {
                        struct minmea_time time;
                        struct minmea_date date;
                        int hour_offset;
                        int minute_offset;
                    };
                    */
                    setTimeFromGPS(&zda.date, &zda.time);
                }
            break;

        }
    }


    void readGPS(void *pvParameters)
    {
        printf("readGPS starting \n");
        GPS *gps = reinterpret_cast<GPS*>(pvParameters);
        assert(gps != NULL);

        
        uint8_t buffer[128];
        memset(buffer, 0, 128 * sizeof(uint8_t));

        typedef enum 
        {
            SCAN_FIRST_CHAR,
            READ_SENTENCE, 
            PROCESS_SENTENCE
        } readState;

        readState state = SCAN_FIRST_CHAR;
        int len = 0;
        int pos = 0;

        while(1)
        {
            switch(state)
            {
                case SCAN_FIRST_CHAR:
                    //Read first char
                    len = gps->read_uart(buffer, 1);
                    if (len == 1 && buffer[0] == '$')
                    {
                        state = READ_SENTENCE;
                        pos = 1;
                    }
                break;

                case READ_SENTENCE:
                    //Read first char
                    len = gps->read_uart(&buffer[pos], 1);
                    if (len == 1 && pos < 128)
                    {
                        if ( buffer[pos] != '\n')
                        {
                            pos++;
                        }
                        else
                        {
                            buffer[++pos] = '\0';
                            state = PROCESS_SENTENCE;
                        }
                    }
                    else
                    {
                        printf("error pos : %i", pos);
                        state = SCAN_FIRST_CHAR;
                    }
                break;

                case PROCESS_SENTENCE:
                    
                    if (minmea_check((const char*) buffer, true))
                    {
                        //printf("Found sentence : %s\n", buffer);
                        process_sentence((const char*) buffer);
                    }
                    state = SCAN_FIRST_CHAR;
                    memset(buffer, 0, 128 * sizeof(uint8_t));
                break;

            }
            
        }
    }
}

GPS* GPS::_instance = NULL;

GPS* GPS::instance()
{
    if (GPS::_instance == NULL)
        GPS::_instance = new GPS();
    return GPS::_instance;
}

GPS::GPS()
    : _port(UART_NUM_1)
{
    setup_uart();
    
    xTaskCreate(&readGPS, "ReadGPS", 4096, this, 8, &_readGPSHandle);
    printf("GPS initialized\n");

}

void GPS::setup_uart()
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .use_ref_tick = 1
    };
    uart_param_config(_port, &uart_config);

    uart_set_pin(_port, PIN_NUM_UART_TX, PIN_NUM_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    esp_err_t err = uart_driver_install(_port, BUFFER_SIZE * 2, 0, 0, NULL, 0);
    if (err == ESP_OK)
        printf("UART Driver installed\n");

}

int GPS::read_uart(uint8_t *buffer, int max_size)
{
    int len = uart_read_bytes(_port, buffer, max_size, 20 / portTICK_RATE_MS);
    return len;
}