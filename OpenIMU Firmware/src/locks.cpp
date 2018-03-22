#include "locks.h"

#include <Arduino.h>

namespace Locks
{
    SemaphoreHandle_t i2cMutex;

    void create()
    {
        i2cMutex = xSemaphoreCreateMutex();
    }
}
