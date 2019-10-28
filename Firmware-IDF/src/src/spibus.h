#ifndef _SPIBUS_H_
#define _SPIBUS_H_

#include "defines.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"

class SPIBus
{
    public:

    SPIBus(spi_host_device_t dev = HSPI_HOST);

    static bool lock(int timeoutMs = portMAX_DELAY);

    static bool unlock();

    static esp_err_t spi_device_transmit(spi_device_handle_t handle, spi_transaction_t *trans);

    protected:

    spi_bus_config_t _buscfg;
    static SemaphoreHandle_t _mutex;
    
};

#endif