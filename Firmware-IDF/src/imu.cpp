
#include "imu.h"

IMU* IMU::_instance = NULL;

IMU::IMU()
    : _mpu9250(I2C_NUM_1, MPU9250_I2C_ADDRESS)
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

        //_mpu9250.calibrateAccel();
        //_mpu9250.calibrateGyro();
        //_mpu9250.calibrateMag();

        // Enable FIFO
        //_mpu9250.enableFifo(true, true, true, true);

        printf("IMU initialized\n");
    }
    else
    {
        printf("ERROR initializing IMU\n");
    }
}


IMU* IMU::instance()
{
    if (IMU::_instance == NULL)
        IMU::_instance = new IMU();
    return IMU::_instance;
}