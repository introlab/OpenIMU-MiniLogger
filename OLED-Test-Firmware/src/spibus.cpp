#include "spibus.h"
#include <string.h>

SemaphoreHandle_t SPIBus::_mutex = NULL;

SPIBus::SPIBus(spi_host_device_t dev)
{
    memset(&_buscfg, 0, sizeof(spi_bus_config_t));
    _buscfg.miso_io_num = PIN_NUM_MISO;
    _buscfg.mosi_io_num = PIN_NUM_MOSI;
    _buscfg.sclk_io_num = PIN_NUM_CLK;
    _buscfg.quadwp_io_num = -1;
    _buscfg.quadhd_io_num = -1;
    _buscfg.max_transfer_sz = 1000;

    //Needed for adequate SPI pin configuration
    gpio_set_direction((gpio_num_t)PIN_NUM_MISO, GPIO_MODE_INPUT);


    //Initialize the SPI bus data structure, no DMA for now
    esp_err_t ret = spi_bus_initialize(dev, &_buscfg, 0);
    printf("spi_bus_initialize ret: %i\n", ret);
    assert(ret == ESP_OK);

    SPIBus::_mutex = xSemaphoreCreateMutex();
    assert(SPIBus::_mutex != NULL);
}

esp_err_t SPIBus::spi_device_transmit(spi_device_handle_t handle, spi_transaction_t *trans)
{
    //Protect the calls to device transmit
    SPIBus::lock();
    esp_err_t ret = ::spi_device_transmit(handle, trans);
    SPIBus::unlock();
    return ret;
}

bool SPIBus::lock(int timeoutMs)
{
    assert(SPIBus::_mutex != NULL);
    bool success = xSemaphoreTake(SPIBus::_mutex, timeoutMs / portTICK_RATE_MS) == pdTRUE;
    if(!success)
        printf("Coulnd not acquire spi mutex");
    return success;
}

bool SPIBus::unlock()
{
    assert(SPIBus::_mutex != NULL);
    return xSemaphoreGive(SPIBus::_mutex) == pdTRUE;
}