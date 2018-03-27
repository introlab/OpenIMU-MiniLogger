#include "i2cmutex.h"

SemaphoreHandle_t I2CMutex::_i2cMutex = NULL;

I2CMutex::I2CMutex()
{
    if(_i2cMutex == NULL) {
        _i2cMutex = xSemaphoreCreateMutex();
        assert(_i2cMutex != NULL);
    }
}

I2CMutex::~I2CMutex()
{

}

bool I2CMutex::acquire(int timeoutMs)
{
    bool succes = xSemaphoreTake(_i2cMutex, timeoutMs / portTICK_RATE_MS) == pdTRUE;
    if(!succes)
        Serial.println("Coulnd not acquire i2c");
    return succes;
}

bool I2CMutex::release()
{
    return xSemaphoreGive(_i2cMutex) == pdTRUE;
}
