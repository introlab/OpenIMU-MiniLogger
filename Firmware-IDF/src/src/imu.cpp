#include "imu.h"
#include "sdcard.h"

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

        TickType_t lastPrint = xTaskGetTickCount();
        TickType_t now;

        while(1)
        {
            xSemaphoreTake(imu->getSemaphore(), portMAX_DELAY);
            //printf("should read from task\n");
            imuDataPtr_t data = (imuDataPtr_t) malloc(sizeof(imuData_t));
            imu->readSensor(data);

            //Print at 4 Hz for debug
            /*
            now = xTaskGetTickCount();
            if (now - lastPrint > 250 / portTICK_RATE_MS)
            {
                printf("\nAccel : %.2f\t%.2f\t%.2f\n", data->accelX, data->accelY, data->accelZ);
                printf("Mag   : %.2f\t%.2f\t%.2f\n", data->magX, data->magY, data->magZ);
                printf("Gyro  : %.2f\t%.2f\t%.2f\n", data->gyroX, data->gyroY, data->gyroZ);
                lastPrint = now;
            }
            */

            //Send to queue, delete if not working
            if (!SDCard::instance()->enqueue(data))
                free(data);


        }
    }

}



IMU::IMU()
    : _mpu9250(), _readIMUHandle(NULL)
{
    IMUconfig_Sd SdConfig={100,500,8};

    SDCard::instance()->GetConfigFromSd(&SdConfig);

    printf("rate:%d\n",SdConfig.IMUSampleRate);
    printf("Acel:%d\n",SdConfig.IMUAcellRange); 
    printf("Gyro:%d\n",SdConfig.IMUGyroRange);
 

    if (_mpu9250.begin() == 0)
    {
        //IMUconfig_Sd SdConfig={100,500,8};

        if(SDCard::instance()->GetConfigFromSd(&SdConfig))
        {
            setIMUParameter(SdConfig.IMUSampleRate,SdConfig.IMUAcellRange,SdConfig.IMUGyroRange);
        }

        else
        {
            setIMUParameter(SdConfig.IMUSampleRate,SdConfig.IMUAcellRange,SdConfig.IMUGyroRange);
            /*
            _mpu9250.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_41HZ);
            // setting SRD to 9 for a 100 Hz update rate
            // FS is 1 kHz / (SRD + 1)

            _mpu9250.setSrd(9); //100Hz
            //Setting Range
            _mpu9250.setAccelRange(MPU9250::ACCEL_RANGE_8G);
            _mpu9250.setGyroRange(MPU9250::GYRO_RANGE_1000DPS);
            */
        }

        _mpu9250.dmpBegin(DMP_FEATURE_PEDOMETER);
        _mpu9250.dmpSetPedometerSteps(StepCount);
        _mpu9250.dmpSetPedometerTime(StepTime);

        printf("initialisation\n");
        //Does not work...
        //_mpu9250.calibrateAccel();
        //_mpu9250.calibrateGyro();
        //_mpu9250.calibrateMag();

        // Enable FIFO
        //_mpu9250.enableFifo(true, true, true, true);

        
        // Setup interrupt handling
        _semaphore = xSemaphoreCreateCounting(1,0);
        setup_interrupt_pin();
        _mpu9250.enableInterrupt();
        _mpu9250.setIntLevel(INT_ACTIVE_LOW);

        //Create reading task
        xTaskCreatePinnedToCore(&readIMU, "IMU read task", 2048, this, 10, &_readIMUHandle, 0);
        printf("IMU initialized\n");
    }
    else
    {
        printf("ERROR initializing IMU\n");
    }
}

void IMU::readSensor(imuDataPtr_t data)
{
    _mpu9250.update();

    if (data)
    {
        data->accelX = _mpu9250.calcAccel(_mpu9250.ax);
        data->accelY = _mpu9250.calcAccel(_mpu9250.ay);
        data->accelZ = _mpu9250.calcAccel(_mpu9250.az);
        data->gyroX  = _mpu9250.calcGyro(_mpu9250.gx);
        data->gyroY  = _mpu9250.calcGyro(_mpu9250.gy);
        data->gyroZ  = _mpu9250.calcGyro(_mpu9250.gz);
        data->magX = _mpu9250.calcMag(_mpu9250.mx);
        data->magY = _mpu9250.calcMag(_mpu9250.my);
        data->magZ = _mpu9250.calcMag(_mpu9250.mz);

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

void IMU::setSampleRate(int rateHz)
{
    if (rateHz==10)
    {
        _mpu9250.setLPF(5);
        _mpu9250.setSampleRate(10);
        _mpu9250.setCompassSampleRate(10);
        printf("Set sample Rate:10Hz\n");

        SampleRateHz=10;
    }
    else if (rateHz==50)
    {
        _mpu9250.setLPF(20);
        _mpu9250.setSampleRate(50);
        _mpu9250.setCompassSampleRate(50);
        printf("Set sample Rate:50Hz\n");

        SampleRateHz=50;
    }
    else if (rateHz==100)
    {
        _mpu9250.setLPF(42);
        _mpu9250.setSampleRate(100);
        _mpu9250.setCompassSampleRate(100);
        printf("Set sample Rate:100Hz\n");

        SampleRateHz=100;
    }
    else if (rateHz==200)
    {
        _mpu9250.setLPF(98);
        _mpu9250.setSampleRate(200);
        _mpu9250.setCompassSampleRate(100);
        printf("Set sample Rate:200Hz\n");

        SampleRateHz=200;
    }
    else
    {
        _mpu9250.setLPF(42);
        _mpu9250.setSampleRate(100);
        _mpu9250.setCompassSampleRate(100);
        printf("Wrong rate entered,setting to 100Hz");

        SampleRateHz=100;
    }
    
}

void IMU::setIMUParameter(int rateHZ,int accelRange, int gyroRange)
{

    setSampleRate(rateHZ);

    if(accelRange==2 || accelRange==4 || accelRange==8 || accelRange==16)
        _mpu9250.setAccelFSR(accelRange);
    else
    {
        printf("Wrong Acceleration range entered, Setting to 4G");
        _mpu9250.setAccelFSR(4);
    }

    if(gyroRange==250 || gyroRange==500 || gyroRange==1000 || gyroRange==2000)
        _mpu9250.setGyroFSR(gyroRange);
    else
    {
        printf("Wrong Gyro range entered, Setting to 500dps");
        _mpu9250.setGyroFSR(500);
    }
  
}

int IMU::getSampleRate()
{
    int id = 0;
    if (SampleRateHz==10)
        id = 1;
    else if (SampleRateHz==50)
        id = 2;
    else if (SampleRateHz==100)
        id = 3;
    else if (SampleRateHz==200)
        id = 4;

    return id;
}

int IMU::getStepCount()
{
    return _mpu9250.dmpGetPedometerSteps();
}
    
int IMU::getStepTime()
{
    return _mpu9250.dmpGetPedometerTime()/1000;
}