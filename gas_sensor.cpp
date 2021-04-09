
#include "gas_sensor.h"
#include "temp_humidity_sensor.h"
//extern mbed::I2C i2c;
void gas_sensor(int addr,char config)
{
 //   char config;
 //   config = MCP3423_CH0_CONFIG_G1;
    char readbuff[3];
    i2c.write(addr << 1, &config, 1);
    i2c.read(addr << 1, readbuff, 3);
    printf("gas :");
    for(int i=0; i<3 ;i++)
    {
  //      sensor_buffer_MCP0[i + sensordata_count] = readbuff[i];
        printf("%x ", readbuff[i]);
    }
    
    printf("\n");
  /*  config = MCP3423_CH0_CONFIG_G2;
    i2c.write(addr << 1, &config, 1);
    i2c.read(addr << 1, readbuff, 3);
    for(int i=0; i<3 ;i++)
    printf("gas :%x ", readbuff[i]);
    config = MCP3423_CH0_CONFIG_G4;
    i2c.write(addr << 1, &config, 1);
    i2c.read(addr << 1, readbuff, 3);
    for(int i=0; i<3 ;i++)
    printf("gas :%x ", readbuff[i]);
    config = MCP3423_CH0_CONFIG_G8;
    i2c.write(addr << 1, &config, 1);
    i2c.read(addr << 1, readbuff, 3);
    for(int i=0; i<3 ;i++)
    printf("gas :%x ", readbuff[i]);*/
}

void channelwrite(int addr,char config)
{
    i2c.write(addr << 1, &config, 1);
}
uint8_t count1=0,count2=0,count3=0,count4=0;
void channelread(int addr,char config,uint8_t sensordata_count) {
    char readbuff[4];
  //  i2c.write(addr << 1, &config, 1);
    i2c.read(addr << 1, readbuff, 4);
    printf("gas :");
int i=0;
    switch(sensordata_count)
    {

        case 0: 
        printf("m0 ch0 : ");
        for(i=0; i < 3 ;i++) {
            sensor_buffer_MCP0[ i + (3 * count1)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf(" %x\n",readbuff[3]);
        count1++;
        break;
        case 1:
        printf("m0 ch1 : ");
        for(int i=0; i < 3 ;i++) {
            sensor_buffer_MCP1[ i + (3 * count2)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf(" %x\n",readbuff[3]);
        count2++;
        break;
        case 2:
        
        printf("m1 ch0 : ");
        for(int i=0; i < 3 ;i++) {
            sensor_buffer_MCP2[ i + (3 * count3)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf(" %x\n",readbuff[3]);
        count3++;
        break;
        case 3:
        printf("m1 ch1 : ");
        for(int i=0; i < 3 ;i++) {
            sensor_buffer_MCP3[ i + (3 * count4)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf(" %x\n",readbuff[3]);
        count4++;
        break;
        default:
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