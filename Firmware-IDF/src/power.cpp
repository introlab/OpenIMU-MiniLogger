#include "power.h"
#include "sdcard.h"
#include "ioexpander.h"

Power* Power::_instance = NULL;

namespace 
{
    void powerTask(void *pvParameters)
    {
        printf("powerTask starting...\n");
        Power* power = reinterpret_cast<Power*>(pvParameters);
        assert(power);

        //Initialize last tick
        TickType_t _lastTick = xTaskGetTickCount();

        while(1)
        {
            //1Hz
            vTaskDelayUntil(&_lastTick, 1000 / portTICK_RATE_MS);

            powerDataPtr_t data = (powerDataPtr_t) malloc(sizeof(powerData_t));
            //Fill data
            data->voltage = power->read_voltage();
            data->current = power->read_current();
            // Update charging state
            power->read_charging();

            //Send to logging thread
            if (!SDCard::instance()->enqueue(data))
                free(data);
        }
    }
}


Power* Power::instance()
{
    if (Power::_instance == NULL)
        Power::_instance = new Power();
    return Power::_instance;
}

Power::Power()
    : _ads1015(I2C_NUM_1), _last_voltage(0), _last_current(0)
{
    _mutex = xSemaphoreCreateMutex();
    assert(_mutex != NULL);
    IOExpander::instance().pinMode(EXT_PIN10_CHARGING, INPUT);
    IOExpander::instance().pinMode(EXT_PIN14_EXTERNAL_POWER_EN, OUTPUT);
    disableExternalPower();
    xTaskCreate(&powerTask, "PowerTask", 2048, this, 6, &_powerTaskHandle);
}

void Power::enableExternalPower()
{
    IOExpander::instance().digitalWrite(EXT_PIN14_EXTERNAL_POWER_EN, HIGH);
}

void Power::disableExternalPower()
{
    IOExpander::instance().digitalWrite(EXT_PIN14_EXTERNAL_POWER_EN, LOW);
}

float Power::last_voltage()
{
    return _last_voltage;
}

float Power::read_voltage()
{   
    lock();
    uint16_t value = _ads1015.readADC_SingleEnded(ADC_VOLTAGE_CHANNEL);
    unlock();
    //printf("VOLTAGE HEX: %4.4x, %i\n", value, value);
    _last_voltage = 5.0 * 0.002 * (float) value;
    return _last_voltage;
}

float Power::last_current()
{
    return _last_current;
}

float Power::read_current()
{
    lock();
    uint16_t value = _ads1015.readADC_SingleEnded(ADC_CURRENT_CHANNEL);
    unlock();
    //printf("CURRENT HEX: %4.4x, %i\n", value, value);
    _last_current = ((0.002 * (float) value) - (3.3/2.0)) / 5.0;
    return _last_current;
}

bool Power::last_charging()
{
    return _last_charging;
}

bool Power::read_charging()
{
    lock();
    unsigned int value = IOExpander::instance().digitalRead(EXT_PIN10_CHARGING);
    unlock();
    _last_charging = !(bool)value;
    return _last_charging;
}

void Power::lock()
{
    assert(xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE);
}

void Power::unlock()
{
    assert(xSemaphoreGive(_mutex) == pdTRUE);
}