#include "adc.h"
#include "i2cmutex.h"
#include "defines.h"


namespace
{
    Adafruit_ADS1015 _adc;
    I2CMutex _i2c;

    TaskHandle_t _serialLogHangle = NULL;
    TaskHandle_t _queueLogHandle = NULL;

    QueueHandle_t _loggingQueue = NULL;
    SemaphoreHandle_t _sdDataSemaphore = NULL;

    void logSerial(void *pvParameters);
    void logQueue(void *pvParameters);

}

ADC::ADC()
{

}

ADC::~ADC()
{

}

void ADC::begin()
{
    //Enable interrupt / ready for ADC
    pinMode(ADC_READY_PIN, INPUT_PULLUP);

    //Enable battery readings...
    ioExpander.pinMode(EXT_PIN13_BATT_READ_EN, OUTPUT);
    ioExpander.digitalWrite(EXT_PIN13_BATT_READ_EN, HIGH);

    _adc.begin();
}

void ADC::startSerialLogging()
{
    Serial.println("ADC log begin");
    if(_serialLogHangle == NULL) {
        Serial.println("ADC task create");
        xTaskCreate(&logSerial, "ADC serial log", 2048, NULL, 5, &_serialLogHangle);
    }
}

void ADC::stopSerialLogging()
{
    Serial.println("ADC log stop");
    if(_serialLogHangle != NULL) {
        Serial.println("ADC task delete");
        vTaskDelete(_serialLogHangle);
        _serialLogHangle = NULL;
    }
}

void ADC::startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore)
{
    Serial.println("ADC log begin");
    if(_queueLogHandle == NULL) {
        Serial.println("ADC task create");
        _loggingQueue = queue;
        _sdDataSemaphore = semaphore;
        xTaskCreate(&logQueue, "ADC queue log", 2048, NULL, 5, &_queueLogHandle);
    }
}

void ADC::stopQueueLogging()
{
    Serial.println("ADC log stop");
    if(_queueLogHandle != NULL) {
        Serial.println("ADC task delete");
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

                //Read sensor
                int val0 = _adc.readADC_SingleEnded(0);
                int val1 = _adc.readADC_SingleEnded(1);
                int val2 = _adc.readADC_SingleEnded(2);

                _i2c.release();

                // Send data to serial
                //Serial.printf("ADC: %i %i %i \n", val0,val1,val2);

                //VBAT  = VBAT_READ * 5 (2mV per bit)
                //CURRENT = (CSA_READ - VCC/2) / 100
                float vbat = 5.0 * 0.002 * (float) val1;
                float current = (0.002 * (float) val2 - 2.048) / 100.0;
                Serial.printf("ADC VBAT: %f CURRENT: %f\n", vbat, current);

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
