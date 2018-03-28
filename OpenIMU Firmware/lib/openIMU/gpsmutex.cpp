#include "gpsmutex.h"

SemaphoreHandle_t GPSMutex::_gpsMutex = NULL;

GPSMutex::GPSMutex()
{
    if(_gpsMutex == NULL) {
        _gpsMutex = xSemaphoreCreateMutex();
        assert(_gpsMutex != NULL);
    }
}

GPSMutex::~GPSMutex()
{

}

bool GPSMutex::acquire(int timeoutMs)
{
    bool succes = xSemaphoreTake(_gpsMutex, timeoutMs / portTICK_RATE_MS) == pdTRUE;
    if(!succes)
        Serial.println("Coulnd not acquire gps");
    return succes;
}

bool GPSMutex::release()
{
    return xSemaphoreGive(_gpsMutex) == pdTRUE;
}
