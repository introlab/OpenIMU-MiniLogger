#ifndef LOCKS_H
#define LOCKS_H

#include <Arduino.h>

namespace Locks
{
    extern SemaphoreHandle_t i2cMutex;

    extern void create();
}

#endif
