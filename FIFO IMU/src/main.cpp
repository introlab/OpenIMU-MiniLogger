#include <Arduino.h>
#include <SparkFunMPU9250-DMP.h>

#define INTERRUPT_PIN 34
#define PACKET_SIZE 12

MPU9250_DMP mpu9250;
SemaphoreHandle_t imuSemaphore;

void IRAM_ATTR imuInterrupt() {
    xSemaphoreGiveFromISR(imuSemaphore, NULL);
}

void setup() {
    // Init serial port
    Serial.begin(115200);
    while(!Serial) {
        delay(100);
    }
    delay(1000);
    Serial.print("ESP Ready... ");

    // Init imu
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    Wire.begin();

    while (mpu9250.begin() != INV_SUCCESS) {
        Serial.println();
        Serial.println("Unable to communicate with MPU-9250");
        Serial.print("Check connections, and try again.");
        delay(500);
    }
    Serial.println("IMU OK!");

    // Enable all sensors:
    mpu9250.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);

    mpu9250.setGyroFSR(2000); // Set gyro to 2000 dps
    mpu9250.setAccelFSR(2); // Set accel to +/-2g
    mpu9250.setLPF(5); // Set LPF corner frequency to 5Hz

    mpu9250.setSampleRate(100); // Set sample rate to 500Hz
    if(mpu9250.setCompassSampleRate(10) != INV_SUCCESS) // Set mag rate to 10Hz
        Serial.print("Compass rate error. ");
    Serial.printf("Actual compass rate is %d\n", mpu9250.getCompassSampleRate());

    mpu9250.configureFifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);

    // Init interrupt
    mpu9250.setIntLevel(INT_ACTIVE_LOW);
    mpu9250.setIntLatched(INT_LATCHED);

    imuSemaphore = xSemaphoreCreateBinary();
    attachInterrupt(INTERRUPT_PIN, imuInterrupt, FALLING);

    mpu9250.resetFifo();
    mpu9250.enableInterrupt();
}

void loop() {
    unsigned short fifoCount;
    unsigned char nSample, iSample;

    // Wait for new IMU data
    xSemaphoreTake(imuSemaphore, portMAX_DELAY);
    //Serial.print("Got interrupt. ");

    // Check fifo count and read if needed
    fifoCount = mpu9250.fifoAvailable();

    //if(fifoCount > PACKET_BUFFER_SIZE * PACKET_SIZE) {
    if(true) {
        
        // Batch read data
        nSample = PACKET_BUFFER_SIZE;
        mpu9250.batchUpdateFifo(nSample);

        // Batch print data
        Serial.print("Acc X :");
        for(iSample = 0; iSample < nSample; iSample++) {
            Serial.printf("\t% 04.2f", mpu9250.calcAccel(mpu9250.axs[iSample]));
        }

        // Update and print mag
        if(mpu9250.updateCompass() != INV_SUCCESS)
            Serial.println("Compass error");
        Serial.printf("\tCompass X : % 04.2f", mpu9250.calcMag(mpu9250.mx));
        Serial.println();
    }
}