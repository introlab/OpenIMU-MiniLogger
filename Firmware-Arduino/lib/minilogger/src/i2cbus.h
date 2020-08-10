#ifndef _I2CBUS_H_
#define _I2CBUS_H_

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/i2c.h"

class I2CBus
{
    public:

    I2CBus(i2c_port_t dev = I2C_NUM_1);

    static bool lock(int timeoutMs = portMAX_DELAY);

    static bool unlock();

    static esp_err_t i2c_master_cmd_begin(i2c_cmd_handle_t cmd, TickType_t timeout = portMAX_DELAY);

    protected:

    static i2c_port_t _port;
    static i2c_config_t _buscfg;
    static SemaphoreHandle_t _mutex;
    
};

#endif