#include "imu.h"

#include "locks.h"

namespace
{
    MPU9250_DMP _imu;

    TaskHandle_t _serialLogHangle = NULL;
    TaskHandle_t _queueLogHandle = NULL;

    QueueHandle_t _loggingQueue = NULL;

    void printIMUData();
    IMUDataPoint* createIMUDataPoint();

    void logSerial(void *pvParameters);
    void logQueue(void *pvParameters);
}

IMU::IMU()
{

}

IMU::~IMU()
{

}

void IMU::begin()
{
    while (_imu.begin() != INV_SUCCESS) {
        Serial.println("Unable to communicate with MPU-9250");
        Serial.println("Check connections, and try again.");
        Serial.println();
        delay(500);
    }

    // Enable all sensors:
    _imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);

    _imu.setGyroFSR(2000); // Set gyro to 2000 dps
    _imu.setAccelFSR(2); // Set accel to +/-2g
    _imu.setLPF(5); // Set LPF corner frequency to 5Hz

    _imu.setSampleRate(10); // Set sample rate to 10Hz
    _imu.setCompassSampleRate(10); // Set mag rate to 10Hz
}

void IMU::startSerialLogging()
{
    Serial.println("IMU log begin");
    if(_serialLogHangle == NULL) {
        Serial.println("IMU task create");
        xTaskCreate(&logSerial, "IMU serial log", 2048, NULL, 5, &_serialLogHangle);
    }
}

void IMU::stopSerialLogging()
{
    Serial.println("IMU log stop");
    if(_serialLogHangle != NULL) {
        Serial.println("IMU task delete");
        vTaskDelete(_serialLogHangle);
        _serialLogHangle = NULL;
    }
}

void IMU::startQueueLogging(QueueHandle_t queue)
{
    Serial.println("IMU log begin");
    if(_queueLogHandle == NULL) {
        Serial.println("IMU task create");
        _loggingQueue = queue;
        xTaskCreate(&logQueue, "IMU queue log", 2048, NULL, 5, &_queueLogHandle);
    }
}

void IMU::stopQueueLogging()
{
    Serial.println("IMU log stop");
    if(_queueLogHandle != NULL) {
        Serial.println("IMU task delete");
        vTaskDelete(_queueLogHandle);
        _queueLogHandle = NULL;
    }
}

namespace
{
    void printIMUData()
    {
      // After calling update() the ax, ay, az, gx, gy, gz, mx,
      // my, mz, time, and/or temerature class variables are all
      // updated. Access them by placing the object. in front:

      // Use the calcAccel, calcGyro, and calcMag functions to
      // convert the raw sensor readings (signed 16-bit values)
      // to their respective units.
      IMUDataPoint* measure = createIMUDataPoint();

      Serial.println("Accel: " + String(measure->data.accelX) + ", " + String(measure->data.accelY) + ", " + String(measure->data.accelZ) + " g");
      Serial.println("Gyro: " + String(measure->data.gyroX) + ", " + String(measure->data.gyroY) + ", " + String(measure->data.gyroZ) + " dps");
      Serial.println("Mag: " + String(measure->data.magX) + ", " + String(measure->data.magY) + ", " + String(measure->data.magZ) + " uT");
      Serial.println("Time: " + String(_imu.time) + " ms");
      Serial.println();

      delete(measure);
    }

    IMUDataPoint* createIMUDataPoint()
    {
        IMUDataPoint* measure = (IMUDataPoint*)malloc(sizeof(IMUDataPoint));

        measure->data.accelX = _imu.calcAccel(_imu.ax);
        measure->data.accelY = _imu.calcAccel(_imu.ay);
        measure->data.accelZ = _imu.calcAccel(_imu.az);
        measure->data.gyroX = _imu.calcGyro(_imu.gx);
        measure->data.gyroY = _imu.calcGyro(_imu.gy);
        measure->data.gyroZ = _imu.calcGyro(_imu.gz);
        measure->data.magX = _imu.calcMag(_imu.mx);
        measure->data.magY = _imu.calcMag(_imu.my);
        measure->data.magZ = _imu.calcMag(_imu.mz);

        return measure;
    }

    void logSerial(void *pvParameters)
    {
        while(1) {

            if(xSemaphoreTake(Locks::i2cMutex, 100 / portTICK_RATE_MS) == pdTRUE)
            {
                if ( _imu.dataReady() ) {
                    _imu.update(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS);
                    xSemaphoreGive(Locks::i2cMutex);
                    printIMUData();
                }

                else {
                    xSemaphoreGive(Locks::i2cMutex);
                }

                delay(10);
            }

            else {
                Serial.println("Couln'd read from IMU. i2c was locked.");
            }
        }
    }

    void logQueue(void *pvParameters)
    {
        while(1) {
            if(xSemaphoreTake(Locks::i2cMutex, 100 / portTICK_RATE_MS) == pdTRUE)
            {
                if ( _imu.dataReady() ) {
                    _imu.update(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS);
                    xSemaphoreGive(Locks::i2cMutex);

                    IMUDataPoint* measure = createIMUDataPoint();
                    if(xQueueSend(_loggingQueue, measure, 0) != pdTRUE) {
                        Serial.println("Queue is full! Dropping measure");
                        delete(measure);
                    }
                }

                else {
                    xSemaphoreGive(Locks::i2cMutex);
                }

                delay(10);
            }

            else {
                Serial.println("Couln'd read from IMU. i2c was locked.");
            }
        }
    }
}
