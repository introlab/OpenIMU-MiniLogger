#include "imu.h"

#include "locks.h"

namespace
{
    MPU9250_DMP _imu;
    TaskHandle_t _serialLogHangle = NULL;

    void printIMUData();
    void logSerial(void *pvParameters);
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
      float accelX = _imu.calcAccel(_imu.ax);
      float accelY = _imu.calcAccel(_imu.ay);
      float accelZ = _imu.calcAccel(_imu.az);
      float gyroX = _imu.calcGyro(_imu.gx);
      float gyroY = _imu.calcGyro(_imu.gy);
      float gyroZ = _imu.calcGyro(_imu.gz);
      float magX = _imu.calcMag(_imu.mx);
      float magY = _imu.calcMag(_imu.my);
      float magZ = _imu.calcMag(_imu.mz);

      Serial.println("Accel: " + String(accelX) + ", " + String(accelY) + ", " + String(accelZ) + " g");
      Serial.println("Gyro: " + String(gyroX) + ", " + String(gyroY) + ", " + String(gyroZ) + " dps");
      Serial.println("Mag: " + String(magX) + ", " + String(magY) + ", " + String(magZ) + " uT");
      Serial.println("Time: " + String(_imu.time) + " ms");
      Serial.println();
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
}
