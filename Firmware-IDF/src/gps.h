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
    GPS();

    int read_uart(uint8_t *buffer, int max_size);

    private:
    void setup_uart();
    void setup_gpio();
   
    uart_config_t _uart_config;
    TaskHandle_t _readGPSHandle;
};

#endif