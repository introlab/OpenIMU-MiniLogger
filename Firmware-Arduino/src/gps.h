#ifndef _GPS_H_
#define _GPS_H_

#include "defines.h"
#include "minmea.h"
#include <ctime>
#include <sys/time.h>
#include "driver/uart.h"

#define BUFFER_SIZE 1024
#define MAXLINELENGTH 120

class GPS
{
    public:

    //Singleton
    static GPS* instance();
    

    int read_uart(uint8_t *buffer, int max_size);
    void setup_uart();
    void setFix(bool fix);
    bool getFix(){return _gpsFix;}

    private:
    
    GPS();
    static GPS* _instance;
    uart_port_t _port;
    uart_config_t _uart_config;
    TaskHandle_t _readGPSHandle;
    bool _gpsFix;
};

#endif