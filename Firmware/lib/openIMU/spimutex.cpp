#include "spimutex.h"

SemaphoreHandle_t SPIMutex::_mutex = NULL;

SPIMutex::SPIMutex()
{
    if(_mutex == NULL) {
        _mutex = xSemaphoreCreateMutex();
        assert(_mutex != NULL);
    }
}

SPIMutex::~SPIMutex()
{

}

bool SPIMutex::acquire(int timeoutMs)
{
    bool succes = xSemaphoreTake(_mutex, timeoutMs / portTICK_RATE_MS) == pdTRUE;
    if(!succes)
        Serial.println("Coulnd not acquire spi mutex");
    return succes;
}

bool SPIMutex::release()
{
    return xSemaphoreGive(_mutex) == pdTRUE;
}
