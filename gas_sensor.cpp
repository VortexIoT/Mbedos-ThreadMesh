
#include "gas_sensor.h"
#include "temp_humidity_sensor.h"
//extern mbed::I2C i2c;
uint8_t count1=0,count2=0,count3=0,count4=0; //these count value for test purpose to read ADC 

//Write to the ADC channel
void channelwrite(int addr,char config) {
    i2c.write(addr << 1, &config, 1);
}

//Reads from the ADC channels
void channelread(int addr,char config,uint8_t sensordata_count) {
    char readbuff[4];
  //  i2c.write(addr << 1, &config, 1);
    i2c.read(addr << 1, readbuff, 4);
    printf("gas :");
    int i=0;
    switch(sensordata_count) {
        case 0: printf("ADC0 ch0 : ");  //ADC 0 , channel 0
        for(i=0; i < 3 ;i++) {
            sensor_buffer_MCP0[ i + (3 * count1)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf(" %x\n",readbuff[3]);
        count1++;
        if(count1 == 5)
            count1 = 0;
        break;
        case 1: printf("ADC0 ch1 : ");    //ADC 0 , channel 1
        for(int i=0; i < 3 ;i++) {
            sensor_buffer_MCP1[ i + (3 * count2)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf(" %x\n",readbuff[3]);
        count2++;
        if(count2 == 5)
            count2 = 0;
        break;
        case 2: printf("ADC1 ch0 : ");    //ADC 1 , channel 0
        for(int i=0; i < 3 ;i++) {
            sensor_buffer_MCP2[ i + (3 * count3)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf(" %x\n",readbuff[3]);
        count3++;
        if(count3 == 5)
            count3 = 0;
        break;
        case 3: printf("ADC1 ch1 : ");    //ADC 1 , channel 1
        for(int i=0; i < 3 ;i++) {
            sensor_buffer_MCP3[ i + (3 * count4)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf(" %x\n",readbuff[3]);
        count4++;
        if(count4 == 5)
            count4 = 0;
        break;
        default: count1 = count2 = count3 = count4 = 0;
        printf("error\n");
        break;
    }
   /* if(sensordata_count == sensordata_count_mcp0) {
        for(int i=0; i < 3 ;i++) {
            sensor_buffer_MCP0[ i + (2 * sensordata_count)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf("\n");
    } else if(sensordata_count == sensordata_count_mcp1){
        for(int i=0; i < 2 ;i++) {
            sensor_buffer_MCP1[ i + (2 * sensordata_count)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf("\n");
    }
    else if(sensordata_count == sensordata_count_mcp1){
        for(int i=0; i < 2 ;i++) {
            sensor_buffer_MCP1[ i + (2 * sensordata_count)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf("\n");
    }
    else if(sensordata_count == sensordata_count_mcp1){
        for(int i=0; i < 2 ;i++) {
            sensor_buffer_MCP1[ i + (2 * sensordata_count)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf("\n");
    }*/
}
/*
void ISB0_CH0(void) {
    channelwrite(MCP3423_0_ADDR << 1, MCP3423_CH0_CONFIG_G4);
}

void ISB0_CH1(void) {
    channelwrite(MCP3423_0_ADDR << 1, MCP3423_CH1_CONFIG_G4);
}

void ISB1_CH0(void) {
    channelwrite(MCP3423_1_ADDR << 1, MCP3423_CH0_CONFIG_G4);
}

void ISB1_CH1(char config) {
    channelwrite(MCP3423_1_ADDR << 1, MCP3423_CH1_CONFIG_G4);
}*/