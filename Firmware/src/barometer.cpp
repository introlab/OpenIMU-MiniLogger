#include "barometer.h"
#include "i2cmutex.h"


namespace
{
    Adafruit_MPL115A2 _baro;
    I2CMutex _i2c;

    TaskHandle_t _serialLogHangle = NULL;
    TaskHandle_t _queueLogHandle = NULL;

    QueueHandle_t _loggingQueue = NULL;
    SemaphoreHandle_t _sdDataSemaphore = NULL;

    void logSerial(void *pvParameters);
    void logQueue(void *pvParameters);

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
}

void Barometer::startSerialLogging()
{
    Serial.println("Barometer log begin");
    if(_serialLogHangle == NULL) {
        Serial.println("Barometer task create");
        xTaskCreate(&logSerial, "Barometer serial log", 2048, NULL, 5, &_serialLogHangle);
    }
}

void Barometer::stopSerialLogging()
{
    Serial.println("Barometer log stop");
    if(_serialLogHangle != NULL) {
        Serial.println("Barometer task delete");
        vTaskDelete(_serialLogHangle);
        _serialLogHangle = NULL;
    }
}

void Barometer::startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore)
{
    Serial.println("Barometer log begin");
    if(_queueLogHandle == NULL) {
        Serial.println("Barometer task create");
        _loggingQueue = queue;
        _sdDataSemaphore = semaphore;
        xTaskCreate(&logQueue, "Barometer queue log", 2048, NULL, 5, &_queueLogHandle);
    }
}

void Barometer::stopQueueLogging()
{
    Serial.println("Barometer log stop");
    if(_queueLogHandle != NULL) {
        Serial.println("Barometer task delete");
        vTaskDelete(_queueLogHandle);
        _queueLogHandle = NULL;
        _sdDataSemaphore = NULL;
    }
}

namespace
{
    void logSerial(void *pvParameters)
    {
        while(1) {

            if(_i2c.acquire()) {
                // Prevent the RTOS kernel swapping out the task.
                vTaskSuspendAll();
                
                //Read sensor
                float temp = _baro.getTemperature();
                float pressure = _baro.getPressure();

                // The operation is complete.  Restart the RTOS kernel.
                xTaskResumeAll ();

                _i2c.release();

                // Send data to serial
                printf("Barometer, temp: %f, pressure: %f\n", temp, pressure);

            }
            //Wait 1 second
            delay(1000);
        }
    }

    void logQueue(void *pvParameters)
    {
        while(1) {

            if(_i2c.acquire()) {
                //Read Barometer
                _i2c.release();


                //TODO SEND DATA TO QUEUE FOR LOGGING


            }

            //Wait 1 second
            delay(1000);
        }
    }
}
