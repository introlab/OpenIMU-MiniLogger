#include "pulse.h"
#include "ioexpander.h"
#include <stdio.h>
#include "algorithm.h"
#include <string.h>
#include "sdcard.h"
//Static instance
Pulse* Pulse::_instance = NULL;

namespace
{
    void pulseTask(void *pvParameters)
    {
        printf("pulseTask starting...\n");
        Pulse *pulse = reinterpret_cast<Pulse*>(pvParameters);
        assert(pulse);

        uint32_t ir_buff[MAX30102_FIFO_SIZE+ALGO_BUFFER_SIZE]={0};    
        uint32_t red_buff[MAX30102_FIFO_SIZE+ALGO_BUFFER_SIZE]={0};
        uint32_t * ptr_ir=ir_buff;
        uint32_t * ptr_red=red_buff;
        uint16_t buf_ind=0;
        uint8_t n_elem=0; 

        bool spo2_valid,hr_valid; // Maybe integrate them in pulse struct, depends on OpenIMU soft

        // FILE* f_ir = fopen("/sdcard/test1.dat","w");
        // assert(f_ir);
        // printf("File Open");

        TickType_t lastGeneration = xTaskGetTickCount();
         
        while(1){
            // Careful about the Sampling Frenquency, FIFO can hold MAX30102_FIFO_SIZE values, 
            // call the read function before FIFO go full, no interrupt wired for now
            vTaskDelayUntil(&lastGeneration, 250 / portTICK_RATE_MS);

            // Add measured value to the IR and Red buffers and return added value number, n_elem=<MAX30102_FIFO_SIZE
            pulse->getLedVal(ptr_ir+buf_ind , ptr_red+buf_ind ,&n_elem);
            
            // Update of buffer used index
            buf_ind+=n_elem;
            // printf("Pulse task : n_= %d | buf_ind= %d\n", n_elem,buf_ind);

            if (buf_ind>ALGO_BUFFER_SIZE)
            {
            // Algorithm use more data to calculate hr and spo2 
            // ALGO_BUFFER_SIZE > PULSE_BUFFER_LENGHT
            // PULSE_BUFFER_LENGHT varies data size send to SD card and the frequency of data sending
            buf_ind-=PULSE_BUFFER_LENGHT; 
            
            //Memory allocation of data structure
            pulseDataPtr_t data = (pulseDataPtr_t) malloc(sizeof(pulseData_t));
            //Fill data
            memcpy(data->ir_led_data, ptr_ir+ALGO_BUFFER_SIZE-PULSE_BUFFER_LENGHT, sizeof(int)*100);
            memcpy(data->red_led_data, ptr_red+ALGO_BUFFER_SIZE-PULSE_BUFFER_LENGHT, sizeof(int)*100);
            //  // Debugg : print last PULSE_BUFFER_LENGHT mesured values, 
            //  // take lot of CPU utilisation, values are discontinued because of this
                // printf("\n FIFO \n");
                // for(i=0;i < PULSE_BUFFER_LENGHT; i++){
                // printf("Sample %d : ", i);
                // printf("red: %d || " , data->red_led_data );
                // printf("ir : %d\n" , data->ir_led_data );
                // }
            
            // TO DO : Improve Signal Analysis  
            maxim_heart_rate_and_oxygen_saturation(ptr_ir, ALGO_BUFFER_SIZE, ptr_red, &(data->spo2),&spo2_valid,
                &(data->heartrate),&hr_valid);
            // printf("spo2 : v=%d %f ; hr : v=%d %d",spo2_valid,data->spo2,hr_valid,data->heartrate);


            // Send to logging thread
            if (!SDCard::instance()->enqueue(data))
                free(data);


            memmove(ptr_ir,ptr_ir+PULSE_BUFFER_LENGHT,sizeof(int)*buf_ind);
            memmove(ptr_red,ptr_red+PULSE_BUFFER_LENGHT,sizeof(int)*buf_ind);
            
            // fwrite(data->ir_led_data, sizeof(uint32_t)*100, 1, f_ir);
            // fflush(f_ir);
            // fsync(fileno(f_ir));
            }
            else
            {
                vTaskDelayUntil(&lastGeneration, 50 / portTICK_RATE_MS);
            }
            
        } 
    }
}

Pulse* Pulse::instance()
{
    if (Pulse::_instance == NULL)
        Pulse::_instance = new Pulse();
    return Pulse::_instance;
}

Pulse::Pulse()
    :  _max30102(I2C_NUM_0),_pulseTaskHandle(NULL){
    // Init here
    
    // Detect and create task only if sensor connected, if not plug the sensor and restart the device
    if (_max30102.init_config()!= ESP_OK)
        printf("MAX30102 not connected, check and restart device to use it\n");
    else
        xTaskCreate(&pulseTask, "PulseTask", 16384, this, 5, &_pulseTaskHandle);
    
   

}

Pulse::~Pulse()
{

}

void Pulse::getLedVal(uint32_t * pun_ir_led, uint32_t * pun_red_led,uint8_t *k)
{
     _max30102.readFIFO(pun_red_led,pun_ir_led,k);
}