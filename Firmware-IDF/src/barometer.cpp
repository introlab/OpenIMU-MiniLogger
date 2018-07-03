#include "barometer.h"
#include "sdcard.h"

//Static instance
Barometer* Barometer::_instance = NULL;

namespace
{
    void baroTask(void *pvParameters)
    {
        printf("baroTask starting...\n");
        Barometer* baro = reinterpret_cast<Barometer*>(pvParameters);
        assert(baro);

        //Initialize last tick
        TickType_t _lastTick = xTaskGetTickCount();

        while(1)
        {
            //1Hz
            vTaskDelayUntil(&_lastTick, 1000 / portTICK_RATE_MS);

            //Memory allocation of data structure
            baroDataPtr_t data = (baroDataPtr_t) malloc(sizeof(baroData_t));

            //fill data
            baro->getPT(data->pressure, data->temperature);

            printf("baro : %f %f\n", data->pressure, data->temperature);
            
            //Send to logging thread
            if (!SDCard::instance()->enqueue(data))
                free(data);
            
            
        }
    }
}

Barometer* Barometer::instance()
{
    if (Barometer::_instance == NULL)
        Barometer::_instance = new Barometer();
    return Barometer::_instance;
}

Barometer::Barometer()
    : _mpl115A2(I2C_NUM_1), _baroTaskHandle(NULL)
{
    //Create task
    xTaskCreate(&baroTask, "BaroTask", 2048, this, 5, &_baroTaskHandle);

}

void Barometer::getPT(float &p, float&t)
{
    _mpl115A2.getPT(p,t);
}