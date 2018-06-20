#if 0
#include "imu.h"

IMU::IMU()
    : _imu(I2C_NUM_1, MPU9250_I2C_ADDRESS)
{
    if (_imu.begin())
    {
        _imu.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
        // setting SRD to 9 for a 100 Hz update rate
        // FS is 1 kHz / (SRD + 1)
        _imu.setSrd(19); //50Hz

        //Setting Range
        _imu.setAccelRange(MPU9250::ACCEL_RANGE_8G);
        _imu.setGyroRange(MPU9250::GYRO_RANGE_2000DPS);

        //_imu.calibrateAccel();
        //_imu.calibrateGyro();
        //_imu.calibrateMag();

        // Enable FIFO
        //_imu.enableFifo(true, true, true, true);


    }
    else
    {

    }
}
#endif