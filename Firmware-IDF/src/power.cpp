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
    : _ads1015(I2C_NUM_1)
{
    _mutex = xSemaphoreCreateMutex();
    assert(_mutex != NULL);
    IOExpander::instance().pinMode(EXT_PIN14_EXTERNAL_POWER_EN, OUTPUT);
    disableExternalPower();
    xTaskCreate(&powerTask, "PowerTask", 2048, this, 5, &_powerTaskHandle);
}

void Power::enableExternalPower()
{
    IOExpander::instance().digitalWrite(EXT_PIN14_EXTERNAL_POWER_EN, HIGH);
}

void Power::disableExternalPower()
{
    IOExpander::instance().digitalWrite(EXT_PIN14_EXTERNAL_POWER_EN, LOW);
}

float Power::read_voltage()
{   
    lock();
    uint16_t value = _ads1015.readADC_SingleEnded(ADC_VOLTAGE_CHANNEL);
    unlock();
    //printf("VOLTAGE HEX: %4.4x, %i\n", value, value);
    return 5.0 * 0.002 * (float) value;
}

float Power::read_current()
{
    lock();
    uint16_t value = _ads1015.readADC_SingleEnded(ADC_CURRENT_CHANNEL);
    unlock();
    //printf("CURRENT HEX: %4.4x, %i\n", value, value);
    return ((0.002 * (float) value) - (3.1/2.0)) / 5.0;
}

void Power::lock()
{
    assert(xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE);
}

void Power::unlock()
{
    assert(xSemaphoreGive(_mutex) == pdTRUE);
}