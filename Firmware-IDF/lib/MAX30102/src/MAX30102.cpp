#include "MAX30102.h"

#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

MAX30102::MAX30102(i2c_port_t port, uint8_t address)
    : _port(port), _address(address)
{
    
}


esp_err_t MAX30102::init_config()
{
    // see datasheet to modify register values
    esp_err_t ret= ESP_OK;
    ret|=writeRegister(REG_INTR_ENABLE_1,0xc0); // INTR setting
    ret|=writeRegister(REG_INTR_ENABLE_2,0x00);
    ret|=writeRegister(REG_FIFO_WR_PTR,0x00);  //FIFO_WR_PTR[4:0]
    ret|=writeRegister(REG_OVF_COUNTER,0x00);  //OVF_COUNTER[4:0]
    ret|=writeRegister(REG_FIFO_RD_PTR,0x00);  //FIFO_RD_PTR[4:0]
    ret|=writeRegister(REG_FIFO_CONFIG,0x1f);  //sample avg = 2, fifo rollover=false, fifo almost full = 17
    ret|=writeRegister(REG_MODE_CONFIG,0x03);   //0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
    ret|=writeRegister(REG_SPO2_CONFIG,0x27);  // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
    ret|=writeRegister(REG_LED1_PA,0x24);   //Choose value for ~ 7mA for LED1
    ret|=writeRegister(REG_LED2_PA,0x24);   // Choose value for ~ 7mA for LED2
    ret|=writeRegister(REG_PILOT_PA,0x7f);   // Choose value for ~ 25mA for Pilot LED
    printf("test :)");
    return ret;
}

esp_err_t MAX30102::readFIFO(uint32_t * pun_red_led, uint32_t * pun_ir_led,uint8_t *k)
{    
    // This function have to be called every X seconds to have continued values
    // SMP_AVE / SPO2_SR < X < MAX30102_FIFO_SIZE_1LED * SMP_AVE / SPO2_SR
    
    esp_err_t ret= ESP_OK;
    //read and clear status register, Interrupt no wired atm
    uint8_t status[1];
    ret|=readRegisters(REG_INTR_STATUS_1, 1, status);
    ret|=readRegisters(REG_INTR_STATUS_2, 1, status);

    // Calcul of samples number to be read from FIFO
    uint8_t wr_ptr[1];
    uint8_t rd_ptr[1];
    ret|=readRegisters(REG_FIFO_WR_PTR,1,wr_ptr);
    ret|=readRegisters(REG_FIFO_RD_PTR,1,rd_ptr);
    uint8_t toRead=16;
    toRead = ((wr_ptr[0]- rd_ptr[0]) & (MAX30102_FIFO_SIZE-1));
    *k=toRead;

    // Add the read values to the buffers in parameters
    if (toRead>0)
    {
        uint8_t buffer[6*toRead];
        ret|=readRegisters(REG_FIFO_DATA, 6*toRead, buffer);

        for(int i=0;i<toRead;i++)
        {
            pun_red_led[i]=(buffer[0 + i*6] << 16) | (buffer[1 + i*6] << 8) | (buffer[2 + i*6] << 0);
            pun_red_led[i] &= 0x03FFFF;  //Mask MSB [23:18]
            pun_ir_led[i]=(buffer[3 + i*6] << 16) | (buffer[4 + i*6] << 8) | (buffer[5 + i*6] << 0);
            pun_ir_led[i] &= 0x03FFFF; //Mask MSB [23:18]
        }
    }
    return ret;
    // TO DO : 
    // - make a return when sensor disconnected
    // - Implement Interrupt Wire in MiniLogger new design to avoid polling
    // - Modify function to use only "FIFO Almost Full Flag" as interrupt 
    // instead of "New FIFO Data Ready" to reduce communications and maybe energy consumption
}

esp_err_t MAX30102::readRegisters(uint8_t reg, uint8_t count, uint8_t* dest)
{

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_address) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    //Re-start
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_address) | I2C_MASTER_READ, ACK_CHECK_EN);

    // Last byte should have nack
    for (int i = 0; i < count; i++)
    {
        //Read bytes
        if (i == count -1)
            i2c_master_read_byte(cmd, &dest[i], (i2c_ack_type_t) NACK_VAL);
        else
            i2c_master_read_byte(cmd, &dest[i], (i2c_ack_type_t) ACK_VAL);
    }

    //Stop
    i2c_master_stop(cmd);

    //Send command
    esp_err_t ret = I2CBusExt::i2c_master_cmd_begin(cmd);

    if (ret != ESP_OK)
        printf("ERROR MAX30102::readRegisters I2C Bus: ret = %i\n", ret);
    //Delete cmd
    i2c_cmd_link_delete(cmd);

    //Return result
    return ret;
}

esp_err_t MAX30102::writeRegister(uint8_t reg, uint8_t data)
{

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_address ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    //Send command
    esp_err_t ret = I2CBusExt::i2c_master_cmd_begin(cmd);
    if (ret != ESP_OK)
        printf("ERROR MAX30102::writeRegister I2C Bus: ret = %i\n", ret);

    i2c_cmd_link_delete(cmd);

    //Return result
    return ret;
}