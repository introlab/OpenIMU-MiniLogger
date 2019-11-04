/******************************************************************************
arduino_mpu9250_i2c.cpp - MPU-9250 Digital Motion Processor Arduino Library 
Jim Lindblom @ SparkFun Electronics
original creation date: November 23, 2016
https://github.com/sparkfun/SparkFun_MPU9250_DMP_Arduino_Library

This library implements motion processing functions of Invensense's MPU-9250.
It is based on their Emedded MotionDriver 6.12 library.
	https://www.invensense.com/developers/software-downloads/

Development environment specifics:
Arduino IDE 1.6.12
SparkFun 9DoF Razor IMU M0

Supported Platforms:
- ATSAMD21 (Arduino Zero, SparkFun SAMD21 Breakouts)
******************************************************************************/
#include "arduino_mpu9250_i2c.h"

#include "i2cbus.h"

#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

int arduino_i2c_write(unsigned char slave_addr, unsigned char reg_addr,
                       unsigned char length, unsigned char * data)
{
	//printf("i2c_write %2.2x %2.2x (%i) \n", slave_addr, reg_addr, length);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);

	for (int i = 0; i < length; i++)
    	i2c_master_write_byte(cmd, data[i], ACK_CHECK_EN);

    i2c_master_stop(cmd);

    //Send command
    esp_err_t ret = I2CBus::i2c_master_cmd_begin(cmd);
    i2c_cmd_link_delete(cmd);

	//printf("I2C Write ret: %i\n", ret);


	/*
	Wire.beginTransmission(slave_addr);
	Wire.write(reg_addr);
	for (unsigned char i = 0; i < length; i++)
	{
		Wire.write(data[i]);
	}
	Wire.endTransmission(true);
	*/

	return ret;
}

int arduino_i2c_read(unsigned char slave_addr, unsigned char reg_addr,
                       unsigned char length, unsigned char * data)
{

	//printf("i2c_read %2.2x %2.2x (%i) \n", slave_addr, reg_addr, length);

	/*
	Wire.beginTransmission(slave_addr);
	Wire.write(reg_addr);
	Wire.endTransmission(false);
	Wire.requestFrom(slave_addr, length);
	for (unsigned char i = 0; i < length; i++)
	{
		data[i] = Wire.read();
	}
	*/

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    //Re-start
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_READ, ACK_CHECK_EN);


    for (int i = 0; i < length; i++)
    {
        //Read bytes
        if (i == length -1)
            i2c_master_read_byte(cmd, &data[i], (i2c_ack_type_t) NACK_VAL);
        else
            i2c_master_read_byte(cmd, &data[i], (i2c_ack_type_t) ACK_VAL);
    }
    

    //Stop
    i2c_master_stop(cmd);
    //Send command
    esp_err_t ret = I2CBus::i2c_master_cmd_begin(cmd);

	//printf("I2C Read ret: %i\n", ret);

    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK)
    {
        return 0;
    }
    else
    {
        printf("arduino_i2c_read error \n");
        return -1;
    }

	return ret;
	
}
