/*
FIFO_SPI.ino
Brian R Taylor
brian.taylor@bolderflight.com

Copyright (c) 2017 Bolder Flight Systems

Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, 
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or 
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <Arduino.h>
#include "MPU9250.h"

// an MPU9250 object with the MPU-9250 sensor on I2C bus at address 0x68
MPU9250FIFO IMU(Wire,0x68);
int status;
uint16_t fifoCount;

// an interrupt for the imu data ready
#define INTERRUPT_PIN 34
SemaphoreHandle_t dataSemaphore;

void IRAM_ATTR imuInterrupt() {
    xSemaphoreGiveFromISR(dataSemaphore, NULL);
}

// variables to hold FIFO data, these need to be large enough to hold the data
float ax[100], ay[100], az[100];
float gx[100], gy[100], gz[100];
float mx[100], my[100], mz[100];
float t[100];
size_t fifoSize;

void setup() {
  // serial to display data
  Serial.begin(115200);
  while(!Serial) {}
  delay(2000);

  // start communication with IMU 
  do {
    status = IMU.begin();
    if (status < 0) {
        Serial.println("IMU initialization unsuccessful");
        Serial.println("Check IMU wiring or try cycling power");
        Serial.print("Status: ");
        Serial.println(status);
    }
    delay(500);
  } while( status < 0);
  Serial.println("IMU initialization successful");
  Serial.print("Status: ");
  Serial.println(status);
  
  // setting DLPF bandwidth to 20 Hz
  IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
  // setting SRD to 19 for a 50 Hz update rate
  // FS is 1 kHz / (SRD + 1)
  IMU.setSrd(19);
  // enabling the FIFO to record accel, gyro and compass
  IMU.enableFifo(true,true,true,false);

  // enable interrupt
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  dataSemaphore =  xSemaphoreCreateBinary();
  attachInterrupt(INTERRUPT_PIN, imuInterrupt, FALLING);
  IMU.enableDataReadyInterrupt();
}

void loop() {

    // wait for a new sample
    xSemaphoreTake(dataSemaphore, portMAX_DELAY);
    if(IMU.readFifoCount(fifoCount) < 0)
        Serial.println("Error retreiving fifo count");

    if(fifoCount < 19*10)
        return;

  // read the fifo buffer from the IMU
  IMU.readFifo();
  // get the X, Y, and Z accelerometer data and their size
  IMU.getFifoAccelX_mss(&fifoSize,ax);
  IMU.getFifoAccelY_mss(&fifoSize,ay);
  IMU.getFifoAccelZ_mss(&fifoSize,az);

  IMU.getFifoMagX_uT(&fifoSize, mx);
  IMU.getFifoMagY_uT(&fifoSize, my);
  IMU.getFifoMagZ_uT(&fifoSize, mz);

  IMU.getFifoGyroX_rads(&fifoSize, gx);
  IMU.getFifoGyroY_rads(&fifoSize, gy);
  IMU.getFifoGyroZ_rads(&fifoSize, gz);

  // print the data
  Serial.print("The FIFO buffer is ");
  Serial.print(fifoSize);
  Serial.println(" samples long.");

  Serial.println("Accel data is:");
  for (size_t i=0; i < fifoSize; i++) {
    Serial.print(ax[i]/9.8,6);
    Serial.print("\t");
    Serial.print(ay[i]/9.8,6);
    Serial.print("\t");
    Serial.println(az[i]/9.8,6);
  }

  Serial.println("Mag data is:");
  for (size_t i=0; i < fifoSize; i++) {
    Serial.print(mx[i],6);
    Serial.print("\t");
    Serial.print(my[i],6);
    Serial.print("\t");
    Serial.println(mz[i],6);
  }

  Serial.println("Gyro data is:");
  for (size_t i=0; i < fifoSize; i++) {
    Serial.print(gx[i],6);
    Serial.print("\t");
    Serial.print(gy[i],6);
    Serial.print("\t");
    Serial.println(gz[i],6);
  }
}