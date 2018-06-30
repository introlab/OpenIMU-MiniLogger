#include "MPL115A2.h"

#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */


namespace
{
    void delay(int ms)
    {
        vTaskDelay(ms / portTICK_RATE_MS);
    }
}

MPL115A2::MPL115A2(i2c_port_t port, uint8_t address)
    : _port(port), _address(address)
{
    _mpl115a2_a0 = 0.0F;
    _mpl115a2_b1 = 0.0F;
    _mpl115a2_b2 = 0.0F;
    _mpl115a2_c12 = 0.0F;
    readCoefficients();
}


float MPL115A2::getPressure(void)
{
    float pressure, temperature;
    getPT(pressure, temperature);
    return pressure;
}

float MPL115A2::getTemperature(void)
{
    float pressure, temperature;
    getPT(pressure, temperature);
    return temperature;
}

void MPL115A2::getPT(float &P, float &T)
{
    uint16_t pressure = 0;
    uint16_t temp = 0;
    uint8_t data[4];
    float pressureComp = 0;

    writeRegister(MPL115A2_REGISTER_STARTCONVERSION, 0x00);
    // Wait a bit for the conversion to complete (3ms max)
    delay(10);

    // Read all data
    readRegisters(MPL115A2_REGISTER_PRESSURE_MSB, 4, data);

    pressure = (( (uint16_t) data[0] << 8) | data[1]) >> 6;
    temp = (( (uint16_t) data[2] << 8) | data[3]) >> 6;

    // See datasheet p.6 for evaluation sequence
    pressureComp = _mpl115a2_a0 + (_mpl115a2_b1 + _mpl115a2_c12 * temp ) * pressure + _mpl115a2_b2 * temp;

    // Return pressure and temperature as floating point values
    P = ((65.0F / 1023.0F) * pressureComp) + 50.0F;        // kPa
    T = ((float) temp - 498.0F) / -5.35F +25.0F;           // C

}

void MPL115A2::readCoefficients(void)
{
    coefdata_t coefs;
    uint8_t data[8];
    readRegisters(MPL115A2_REGISTER_A0_COEFF_MSB, 8, data);

    coefs.a0coeff = ((uint16_t) data[0] << 8) | data[1];
    coefs.b1coeff = ((uint16_t) data[2] << 8) | data[3];
    coefs.b2coeff = ((uint16_t) data[4] << 8) | data[5];
    coefs.c12coeff = (((uint16_t) data[6] << 8) | data[7]) >> 2;

    //Update coefs
    _mpl115a2_a0 = (float)coefs.a0coeff / 8;
    _mpl115a2_b1 = (float)coefs.b1coeff / 8192;
    _mpl115a2_b2 = (float)coefs.b2coeff / 16384;
    _mpl115a2_c12 = (float)coefs.c12coeff;
    _mpl115a2_c12 /= 4194304.0;
}

esp_err_t MPL115A2::readRegisters(uint8_t reg, uint8_t count, uint8_t* dest)
{

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    //Re-start
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_READ, ACK_CHECK_EN);

    i2c_master_read(cmd, dest, count, (i2c_ack_type_t) ACK_VAL);
/*
    for (int i = 0; i < count; i++)
    {
        //Read bytes
        if (i == count -1)
            i2c_master_read_byte(cmd, &dest[i], (i2c_ack_type_t) NACK_VAL);
        else
            i2c_master_read_byte(cmd, &dest[i], (i2c_ack_type_t) ACK_VAL);
    }
*/
    //Stop
    i2c_master_stop(cmd);

    //Send command
    esp_err_t ret = I2CBus::i2c_master_cmd_begin(cmd);

    //Delete cmd
    i2c_cmd_link_delete(cmd);

    //Return result
    return ret;
}

esp_err_t MPL115A2::writeRegister(uint8_t reg, uint8_t data)
{

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    //Send command
    esp_err_t ret = I2CBus::i2c_master_cmd_begin(cmd);
    i2c_cmd_link_delete(cmd);

    //Return result
    return ret;
}