#include "i2cbusext.h"
#include <string.h>

SemaphoreHandle_t I2CBusExt::_mutex = NULL;
i2c_port_t I2CBusExt::_port = I2C_NUM_0;
i2c_config_t I2CBusExt::_buscfg;

I2CBusExt::I2CBusExt(i2c_port_t dev)
{
    _port = dev;
     memset(&_buscfg, 0, sizeof(i2c_config_t));
    _buscfg.mode = I2C_MODE_MASTER;
    _buscfg.sda_io_num = (gpio_num_t) PIN_NUM_EXTSDA;
    _buscfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
    _buscfg.scl_io_num = (gpio_num_t) PIN_NUM_EXTSCL;
    _buscfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
    _buscfg.master.clk_speed = 400000;
    i2c_param_config(_port, &_buscfg);

    //Master mode does not need buffers
    esp_err_t ret = i2c_driver_install(_port, _buscfg.mode, 0, 0, 0);
    printf("configure_i2c_ext ret: %i\n", ret);
    assert(ret == ESP_OK);
    I2CBusExt::_mutex = xSemaphoreCreateMutex();
    assert(I2CBusExt::_mutex != NULL);
}

esp_err_t I2CBusExt::i2c_master_cmd_begin(i2c_cmd_handle_t cmd, TickType_t timeout)
{
    esp_err_t ret = ESP_FAIL;

    if (I2CBusExt::lock())
    {
        ret = ::i2c_master_cmd_begin(I2CBusExt::_port, cmd, timeout);
        I2CBusExt::unlock();
        return ret;
    }
    return ret;
}


bool I2CBusExt::lock(int timeoutMs)
{
    assert(I2CBusExt::_mutex != NULL);
    bool success = xSemaphoreTake(I2CBusExt::_mutex, timeoutMs / portTICK_RATE_MS) == pdTRUE;
    if(!success)
        printf("Coulnd not acquire i2c ext mutex");
    return success;
}

bool I2CBusExt::unlock()
{
    assert(I2CBusExt::_mutex != NULL);
    return xSemaphoreGive(I2CBusExt::_mutex) == pdTRUE;
}