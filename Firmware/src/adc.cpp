#include "adc.h"
#include "i2cmutex.h"
#include "defines.h"


namespace
{
    Adafruit_ADS1015 _adc;
    I2CMutex _i2c;

    TaskHandle_t _serialLogHangle = NULL;
    TaskHandle_t _queueLogHandle = NULL;
    TaskHandle_t _readSensorHandle = NULL;

    QueueHandle_t _loggingQueue = NULL;
    SemaphoreHandle_t _sdDataSemaphore = NULL;

    void logSerial(void *pvParameters);
    void logQueue(void *pvParameters);
    void readSensor(void *pvParameters);

}

ADC::ADC()
{
    voltage = 0;
    current = 0;
}

ADC::~ADC()
{
    stopSerialLogging();
    stopQueueLogging();

    if (_readSensorHandle != NULL)
    {
        vTaskDelete(_readSensorHandle);
        _readSensorHandle = NULL;
    }
}

void ADC::begin()
{
    //Enable interrupt / ready for ADC
    pinMode(ADC_READY_PIN, INPUT_PULLUP);

    //Enable battery readings...
    ioExpander.pinMode(EXT_PIN13_BATT_READ_EN, OUTPUT);
    ioExpander.digitalWrite(EXT_PIN13_BATT_READ_EN, HIGH);

    _adc.begin();

    xTaskCreate(&readSensor, "ADC read sensor", 2048, this, 5, &_readSensorHandle);
}

float ADC::getVoltage()
{
  return voltage;
}

float ADC::getCurrent()
{
  return current;
}

void ADC::setVoltage(float v)
{
    voltage = v;
}

void ADC::setCurrent(float c)
{
    current = c;
}


void ADC::startSerialLogging()
{
    Serial.println("ADC log begin");
    if(_serialLogHangle == NULL) {
        Serial.println("ADC task create");
        xTaskCreate(&logSerial, "ADC serial log", 2048, this, 5, &_serialLogHangle);
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
        xTaskCreate(&logQueue, "ADC queue log", 2048, this, 5, &_queueLogHandle);
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

    /**
      This task will always run and is responsible for
      reading and converting ADC values to voltage, current
    */
    void readSensor(void *pvParameters)
    {

      Serial.println("ADC readSensor Task starting...");

      //The pointer is given to ADC instance
      ADC *adc  = (ADC*)(pvParameters);

      while(1) {

          if(_i2c.acquire()) {
              //Read sensor
              //Therm

              int val0 = _adc.readADC_SingleEnded(0);
              //VBATT

              int val1 = _adc.readADC_SingleEnded(1);
              //CSA

              int val2 = _adc.readADC_SingleEnded(2);
              _i2c.release();

              //Data conversion...
              //VBAT  = VBATT_READ * 5 (2mV per bit)
              //CURRENT = (CSA_READ - VCC/2) / 100
              float vbat = 5.0 * 0.002 * (float) val1;
              float current = ((0.002 * (float) val2) - (3.1/2.0)) / 5.0;

              //Store in the instance
              if (adc)
              {
                  //Serial.printf("Setting batt %f %f \n",vbat, current);
                  adc->setVoltage(vbat);
                  adc->setCurrent(current);
              }

          }
          //Wait 1 second
          delay(1000);
      }

    }

    void logSerial(void *pvParameters)
    {
        //The pointer is given to ADC instance
        ADC *adc  = (ADC*)(pvParameters);

        while(1)
        {
            if (adc)
            {
                Serial.printf("ADC VBAT: %f CURRENT: %f\n", adc->getVoltage(), adc->getCurrent());
            }

            //Wait 1 second
            delay(1000);
        }
    }

    void logQueue(void *pvParameters)
    {
        //The pointer is given to ADC instance
        ADC *adc  = (ADC*)(pvParameters);

        while(1)
        {
            if (adc)
            {
                //TODO SEND DATA TO QUEUE
            }

            //Wait 1 second
            delay(1000);
        }
    }
}
