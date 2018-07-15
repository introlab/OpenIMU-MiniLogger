#include "barometer.h"
#include "i2cmutex.h"


namespace
{
    Adafruit_MPL115A2 _baro;
    I2CMutex _i2c;

    SemaphoreHandle_t flagMutex = xSemaphoreCreateMutex();
    TaskHandle_t _readSensorHandle = NULL;

    QueueHandle_t _loggingQueue = NULL;
    SemaphoreHandle_t _sdDataSemaphore = NULL;


    void readSensor(void* pvParameters);

}

Barometer::Barometer()
{

}

Barometer::~Barometer()
{

}

void Barometer::begin()
{
    _baro.begin();

    xTaskCreatePinnedToCore(&readSensor, "Barometer read task", 2048, this, 8, &_readSensorHandle, 0);
}

void Barometer::startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore)
{
    Serial.println("Barometer log begin");
    xSemaphoreTake(flagMutex, portMAX_DELAY);
    _loggingQueue = queue;
    _sdDataSemaphore = semaphore;
    xSemaphoreGive(flagMutex);

}

void Barometer::stopQueueLogging()
{
    Serial.println("Barometer log stop");
    xSemaphoreTake(flagMutex, portMAX_DELAY);
    _loggingQueue = NULL;
    _sdDataSemaphore = NULL;
    xSemaphoreGive(flagMutex);

}

namespace
{

    void readSensor(void* pvParameters)
    {
        Serial.println("Barometer readSensor task starting...");
        while(1) {
            if(_i2c.acquire()) {
                //Read sensor
                float temp = _baro.getTemperature();
                float pressure = _baro.getPressure();
                _i2c.release();

                // Send data to serial
                //printf("Barometer, temp: %f, pressure: %f\n", temp, pressure);

                //Log to SD is enabled ?
                xSemaphoreTake(flagMutex, portMAX_DELAY);
                if (_loggingQueue && _sdDataSemaphore)
                {
                    //Write data from fifo

                    //Allocate new data point
                    //Will be released in SD card.
                    baroData_ptr measure = (baroData_ptr) malloc(sizeof(baroData_t));
                    measure->temperature = temp;
                    measure->pressure = pressure;

                    if(xQueueSend(_loggingQueue, (void *) &measure, 0) != pdTRUE) {
                        Serial.println("Queue is full! Dropping Baro measure");
                        free(measure);
                    }
                    else {
                        //Will wake up writing task
                        xSemaphoreGive(_sdDataSemaphore);
                    }

                }
                xSemaphoreGive(flagMutex);


            }

            //Wait 1 second
            delay(1000);
        } // while(1)
    }

} //namespace
