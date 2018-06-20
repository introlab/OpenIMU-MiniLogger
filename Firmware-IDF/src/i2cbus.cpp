#include "i2cbus.h"

SemaphoreHandle_t I2CBus::_mutex = NULL;

I2CBus::I2CBus(i2c_port_t dev)
    : _port(dev)
{
    _buscfg.mode = I2C_MODE_MASTER;
    _buscfg.sda_io_num = (gpio_num_t) PIN_NUM_SDA;
    _buscfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
    _buscfg.scl_io_num = (gpio_num_t) PIN_NUM_SCL;
    _buscfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
    _buscfg.master.clk_speed = 400000;
    i2c_param_config(_port, &_buscfg);

    //Master mode does not need buffers
    esp_err_t ret = i2c_driver_install(_port, _buscfg.mode, 0, 0, 0);
    printf("configure_i2c ret: %i\n", ret);
    assert(ret == ESP_OK);
    I2CBus::_mutex = xSemaphoreCreateMutex();
    assert(I2CBus::_mutex != NULL);
}

bool I2CBus::lock(int timeoutMs)
{
    assert(I2CBus::_mutex != NULL);
    bool success = xSemaphoreTake(I2CBus::_mutex, timeoutMs / portTICK_RATE_MS) == pdTRUE;
    if(!success)
        printf("Coulnd not acquire i2c mutex");
    return success;
}

bool I2CBus::unlock()
{
    assert(I2CBus::_mutex != NULL);
    return xSemaphoreGive(I2CBus::_mutex) == pdTRUE;
}