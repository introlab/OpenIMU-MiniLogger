
#include "imu.h"

IMU* IMU::_instance = NULL;

namespace
{
    /**
     * Interrupt handler will wake up the readIMU task at every sample (for each axis). 
     */
    void IRAM_ATTR imu_gpio_isr_handler(void* arg)
    {
        IMU *imu = (IMU*) (arg);
        assert(imu != NULL);
        xSemaphoreGiveFromISR(imu->getSemaphore(), NULL);
    }


    void readIMU(void *pvParameters)
    {
        printf("readIMU starting \n");
        IMU *imu = (IMU*) (pvParameters);
        assert(imu != NULL);

        while(1)
        {
            xSemaphoreTake(imu->getSemaphore(), portMAX_DELAY);
            //printf("should read from task\n");
            imuData_t data;
            imu->readSensor(&data);
        }
    }

}



IMU::IMU()
    : _mpu9250(I2C_NUM_1, MPU9250_I2C_ADDRESS), _readIMUHandle(NULL)
{
    if (_mpu9250.begin() == 1)
    {
        _mpu9250.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
        // setting SRD to 9 for a 100 Hz update rate
        // FS is 1 kHz / (SRD + 1)
        _mpu9250.setSrd(19); //50Hz

        //Setting Range
        _mpu9250.setAccelRange(MPU9250::ACCEL_RANGE_8G);
        _mpu9250.setGyroRange(MPU9250::GYRO_RANGE_2000DPS);
        
        //Does not work...
        //_mpu9250.calibrateAccel();
        //_mpu9250.calibrateGyro();
        //_mpu9250.calibrateMag();

        // Enable FIFO
        //_mpu9250.enableFifo(true, true, true, true);

        
        // Setup interrupt handling
        _semaphore = xSemaphoreCreateCounting(1,0);
        setup_interrupt_pin();
        _mpu9250.enableDataReadyInterrupt();

        //Create reading task
        xTaskCreatePinnedToCore(&readIMU, "IMU read task", 2048, this, 10, &_readIMUHandle, 0);
        printf("IMU initialized\n");
    }
    else
    {
        printf("ERROR initializing IMU\n");
    }
}

void IMU::readSensor(imuDataPtr data)
{
    _mpu9250.readSensor();

    if (data)
    {
        data->accelX = _mpu9250.getAccelX_mss() / MPU9250::G;
        data->accelY = _mpu9250.getAccelY_mss() / MPU9250::G;
        data->accelZ = _mpu9250.getAccelZ_mss() / MPU9250::G;
        data->gyroX  = _mpu9250.getGyroX_rads();
        data->gyroY  = _mpu9250.getGyroY_rads();
        data->gyroZ  = _mpu9250.getGyroZ_rads();
        data->magX = _mpu9250.getMagX_uT();
        data->magY = _mpu9250.getMagY_uT();
        data->magZ = _mpu9250.getMagZ_uT();

    }
}

IMU* IMU::instance()
{
    if (IMU::_instance == NULL)
        IMU::_instance = new IMU();
    return IMU::_instance;
}

void IMU::setup_interrupt_pin()
{
    gpio_config_t io_conf;
    //interrupt on falling edge
    io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_NEGEDGE;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO33
    io_conf.pin_bit_mask =  (1ULL << INTERRUPT_PIN);
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t) 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


    //Set ISR
    gpio_isr_handler_add((gpio_num_t)INTERRUPT_PIN, imu_gpio_isr_handler, this);
}

