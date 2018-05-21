#include "adc.h"
#include "i2cmutex.h"
#include "defines.h"


namespace
{
    Adafruit_ADS1015 _adc;
    I2CMutex _i2c;
    SemaphoreHandle_t flagMutex = xSemaphoreCreateMutex();
    TaskHandle_t _readSensorHandle = NULL;
    QueueHandle_t _loggingQueue = NULL;
    SemaphoreHandle_t _sdDataSemaphore = NULL;
    void readSensor(void *pvParameters);
}

ADC::ADC()
{
    voltage = 0;
    current = 0;
}

ADC::~ADC()
{
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

    //This is important to pin the task to core 0, otherwise I2C will be unstable
    xTaskCreatePinnedToCore(&readSensor, "ADC read sensor", 2048, this, 10 , &_readSensorHandle,0);
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

void ADC::startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore)
{
    Serial.println("ADC log begin");
    xSemaphoreTake(flagMutex, portMAX_DELAY);
    _loggingQueue = queue;
    _sdDataSemaphore = semaphore;
    xSemaphoreGive(flagMutex);

}

void ADC::stopQueueLogging()
{
    Serial.println("ADC log stop");
    xSemaphoreTake(flagMutex, portMAX_DELAY);
    _loggingQueue = NULL;
    _sdDataSemaphore = NULL;
    xSemaphoreGive(flagMutex);
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

              //Log to SD is enabled ?
              xSemaphoreTake(flagMutex, portMAX_DELAY);
              if (_loggingQueue && _sdDataSemaphore)
              {
                  //Write data from fifo

                  //Allocate new data point
                  //Will be released in SD card.
                  powerData_ptr measure = (powerData_ptr) malloc(sizeof(powerData_t));
                  measure->voltage = vbat;
                  measure->current = current;
                  
                  if(xQueueSend(_loggingQueue, (void *) &measure, 0) != pdTRUE) {
                      Serial.println("Queue is full! Dropping ADC/Power measure");
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
      }

    }
}
