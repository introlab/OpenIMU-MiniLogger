#include "imu.h"

#include "i2cmutex.h"

#define INTERRUPT_PIN 34
#define MPU9250_I2C_ADDRESS 0x68
#define FIFO_PACKET_SIZE 21 //_fifoFrameSize = accel*6 + gyro*6 + mag*7 + temp*2;
#define FIFO_LENGTH_TARGET 1 // This will change the length of a single read on I2C

namespace
{
    MPU9250 _imu(Wire, MPU9250_I2C_ADDRESS);
    //MPU9250FIFO _imu(Wire, MPU9250_I2C_ADDRESS);
    I2CMutex _i2c;

    TaskHandle_t _serialLogHangle = NULL;
    TaskHandle_t _queueLogHandle = NULL;

    QueueHandle_t _loggingQueue = NULL;
    SemaphoreHandle_t _sdDataSemaphore = NULL;

    void printIMUData(size_t i);
    imuData_ptr createIMUDataPoint(size_t index);

    void logSerial(void *pvParameters);
    void logQueue(void *pvParameters);

    SemaphoreHandle_t _imuReadySemaphore;
    void IRAM_ATTR imuInterrupt() {
        xSemaphoreGiveFromISR(_imuReadySemaphore, NULL);
    }

    // variables to hold FIFO data, these need to be large enough to hold the data
    float ax[100], ay[100], az[100];
    float gx[100], gy[100], gz[100];
    float mx[100], my[100], mz[100];
    size_t fifoSize = 0;
    uint16_t fifoCount = 0;
}

IMU::IMU()
{

}

IMU::~IMU()
{

}

void IMU::begin()
{
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    int count = 0;
    while(_imu.begin() < 0) {
        Serial.println("Unable to communicate with MPU-9250");
        Serial.println("Check connections, and try again.");
        Serial.println();
        Wire.endTransmission();
        delay(500);
        if (++count > 10)
          break;
    }

    _imu.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
    // setting SRD to 9 for a 100 Hz update rate
    // FS is 1 kHz / (SRD + 1)
    _imu.setSrd(19); //50Hz

    //Setting Range
    _imu.setAccelRange(MPU9250::ACCEL_RANGE_8G);
    _imu.setGyroRange(MPU9250::GYRO_RANGE_2000DPS);

    // Enable FIFO
    //_imu.enableFifo(true, true, true, true);

    // Configure interrupt
    _imuReadySemaphore = xSemaphoreCreateBinary();
    attachInterrupt(INTERRUPT_PIN, imuInterrupt, FALLING);

    //Important
    _imu.enableDataReadyInterrupt();
}

void IMU::startSerialLogging()
{
    Serial.println("IMU log begin");
    if(_serialLogHangle == NULL) {
        Serial.println("IMU task create");
        //_imu.readFifo();
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

void IMU::startQueueLogging(QueueHandle_t queue, SemaphoreHandle_t semaphore)
{
    Serial.println("IMU log begin");
    if(_queueLogHandle == NULL) {
        Serial.println("IMU task create");
        _loggingQueue = queue;
        _sdDataSemaphore = semaphore;
        //_imu.readFifo();
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
        _sdDataSemaphore = NULL;
    }
}

namespace
{
    void printIMUData(size_t i)
    {
      // After calling update() the ax, ay, az, gx, gy, gz, mx,
      // my, mz, time, and/or temerature class variables are all
      // updated. Access them by placing the object. in front:

      // Use the calcAccel, calcGyro, and calcMag functions to
      // convert the raw sensor readings (signed 16-bit values)
      // to their respective units.
      imuData_ptr measure = createIMUDataPoint(i);

      Serial.println("Accel: " + String(measure->accelX) + ", " + String(measure->accelY) + ", " + String(measure->accelZ) + " g");
      Serial.println("Gyro: " + String(measure->gyroX) + ", " + String(measure->gyroY) + ", " + String(measure->gyroZ) + " dps");
      Serial.println("Mag: " + String(measure->magX) + ", " + String(measure->magY) + ", " + String(measure->magZ) + " uT");
      Serial.println();

      delete(measure);
    }

    imuData_ptr createIMUDataPoint(size_t index)
    {
        imuData_ptr measure = (imuData_ptr)malloc(sizeof(imuData_t));

        measure->accelX = ax[index] / 9.8;
        measure->accelY = ay[index] / 9.8;
        measure->accelZ = az[index] / 9.8;
        measure->gyroX = gx[index];
        measure->gyroY = gy[index];
        measure->gyroZ = gz[index];
        measure->magX = mx[index];
        measure->magY = my[index];
        measure->magZ = mz[index];

        return measure;
    }

    void logSerial(void *pvParameters)
    {
        while(1) {
            xSemaphoreTake(_imuReadySemaphore, portMAX_DELAY);
    #if 1
            if(_i2c.acquire()) {
              _imu.readSensor();
              _i2c.release();
            }

            //Fill data
            fifoSize = 1;

            ax[0] = _imu.getAccelX_mss();
            ay[0] = _imu.getAccelY_mss();
            az[0] = _imu.getAccelZ_mss();
            gx[0] = _imu.getGyroX_rads();
            gy[0] = _imu.getGyroY_rads();
            gz[0] = _imu.getGyroZ_rads();
            mx[0] = _imu.getMagX_uT();
            my[0] = _imu.getMagY_uT();
            mz[0] = _imu.getMagZ_uT();


            Serial.print("Printing ");
            Serial.print(fifoSize);
            Serial.println(" samples from fifo...");
            for(size_t i = 0; i < fifoSize; i++)
                printIMUData(i);

    #endif

#if 0
            if(_i2c.acquire()) {
                _imu.readFifoCount(fifoCount);
                Serial.printf("Fifo count: %i\n",fifoCount);
                if(fifoCount < FIFO_LENGTH_TARGET * FIFO_PACKET_SIZE) {
                    _i2c.release();
                    continue;
                }
                _imu.readFifo();
                _i2c.release();


                _imu.getFifoAccelX_mss(&fifoSize,ax);
                _imu.getFifoAccelY_mss(&fifoSize,ay);
                _imu.getFifoAccelZ_mss(&fifoSize,az);
                _imu.getFifoMagX_uT(&fifoSize, mx);
                _imu.getFifoMagY_uT(&fifoSize, my);
                _imu.getFifoMagZ_uT(&fifoSize, mz);
                _imu.getFifoGyroX_rads(&fifoSize, gx);
                _imu.getFifoGyroY_rads(&fifoSize, gy);
                _imu.getFifoGyroZ_rads(&fifoSize, gz);

                Serial.print("Printing ");
                Serial.print(fifoSize);
                Serial.println(" samples from fifo...");
                for(size_t i = 0; i < fifoSize; i++)
                    printIMUData(i);


            }
#endif
        } //while(1)


    }

    void logQueue(void *pvParameters)
    {
        while(1) {
            xSemaphoreTake(_imuReadySemaphore, portMAX_DELAY);
            Serial.println("IMU ISR");
            continue;
#if 0

            if(_i2c.acquire()) {
                _imu.readFifoCount(fifoCount);
                if(fifoCount < FIFO_LENGTH_TARGET * FIFO_PACKET_SIZE) {
                    _i2c.release();
                    continue;
                }
                _imu.readFifo();
                _i2c.release();

                _imu.getFifoAccelX_mss(&fifoSize,ax);
                _imu.getFifoAccelY_mss(&fifoSize,ay);
                _imu.getFifoAccelZ_mss(&fifoSize,az);

                _imu.getFifoMagX_uT(&fifoSize, mx);
                _imu.getFifoMagY_uT(&fifoSize, my);
                _imu.getFifoMagZ_uT(&fifoSize, mz);

                _imu.getFifoGyroX_rads(&fifoSize, gx);
                _imu.getFifoGyroY_rads(&fifoSize, gy);
                _imu.getFifoGyroZ_rads(&fifoSize, gz);

                Serial.print("Saving ");
                Serial.print(fifoSize);
                Serial.println(" samples from fifo...");
                for(size_t i = 0; i < fifoSize; i++) {
                    imuData_ptr measure = createIMUDataPoint(i);
                    if(xQueueSend(_loggingQueue, (void *) &measure, 0) != pdTRUE) {
                        Serial.println("Queue is full! Dropping measure");
                        delete(measure);
                    }
                    else {
                        xSemaphoreGive(_sdDataSemaphore);
                    }
                }


            }
#endif
        }


    }
}
