/*
  Oct 2020
*/
#include "temp_humidity_sensor.h"
#include <cstdint>
#include "mbed.h"

//mbed::DigitalOut env_en(SENSE_ENABLE, 1);
#define GAS_ADDR        0x68
/*#define MCP3423_0_WRITE 0xD0
#define MCP3423_0_READ  0xD1
#define MCP3423_1_WRITE 0xD8
#define MCP3423_1_READ  0xD9*/

char mcp23_config_reg[2] = {0x05, 0xB8}; //bank 1, mirror 0 seqop = 1,slewrate disabled HAEN=1 ODR open drain INTPOL active low
//char mcp23_IODIRA[2] = {0x00, 0x00};
char mcp23_IODIRB[2] = {0x10, 0x00};
char mcp_OLATA[2] = {0x0A, 0x03}; //solenoid 1
char mcp_OLATA_solenoid2[2] = {0x0A, 0x0c};
char mcp_OLATA_fan[2] = {0x0A,0xc0};
/*
//have only 2 solenoids so instead 0x3f
char solenoid_on[2] = {0x09,0x0f}; //logic high = 1, logic low = 0 //all 1's means brake so 
char solenoid_off[2] = {0x09,0x00}; //logic high = 1, logic low = 0


char solenoid_reverse[2] = {0x12,0x28}; //logic high = 1, logic low = 0 //all 1's means brake so 
char solenoid_forward[2] = {0x12,0x14}; //logic high = 1, logic low = 0
*/

char mosfet_on[2] = {0x19,0x07}; //logic high = 1, logic low = 0
char mosfet_off[2] = {0x19,0x00}; //logic high = 1, logic low = 0


#define MCP3423_1_CONFIG    0xA0



uint8_t RH_percentage;
uint8_t Temp_centigrade;
EventQueue sensor_eventqueue;
uint8_t si7021eventhanlder;
/*#########################################  I2C Functions Body ###################***********/
// Call this function to set i2c frequency
void i2cinit(void) {
     i2c.frequency(100000);
}

void temp_hum_sensor_read_every_5min(void) {
    si7021eventhanlder = sensor_eventqueue.call_every(60*5000, humidity_temp_read);  //calls every minute can change in future,test purpose 1min
    printf("boot1 ");
}

void set_si7021_datacapture_intervel(int ms) {
    stop_si7021_datacapture();
    si7021eventhanlder = sensor_eventqueue.call_every(ms, humidity_temp_read);  //calls every minute can change in future,test purpose 1min
}
void stop_si7021_datacapture(void) {
    sensor_eventqueue.cancel(si7021eventhanlder);
}

  // Calling this function Reads indoor Rh %  and respective temperature
void humidity_temp_read(void) {//Temp read with RH
    char rhread = HUMIDITY_READ;
    char readbuff[2];
    i2c.write(SENSOR_ADDR<<1,&rhread,1);
    i2c.read(SENSOR_ADDR<<1,readbuff,2);
    uint16_t Rh_reading = (((readbuff[0]<<8)|(readbuff[1]&0xFE)));
    RH_percentage = (((125*Rh_reading)/65536)-6);
    printf("Rhread := %d\n",RH_percentage);
    rhread = TEMP_READ;
    i2c.write(SENSOR_ADDR<<1,&rhread,1);
    i2c.read(SENSOR_ADDR<<1,readbuff,2);
    uint16_t temp_reading = (((readbuff[0]<<8)|(readbuff[1]&0xFC)));
    Temp_centigrade = (((175.72*temp_reading)/65536)-46.85);
    printf("Tempread := %d\n",Temp_centigrade);
}

 // Calling this function Reads indoor temperature
void temperature_sensor_read(void) {//Seperate temp read
    char Temp = TEMPERATURE_READ;
    char readbuff[2];
    i2c.write(SENSOR_ADDR<<1,&Temp,1);
    i2c.read(SENSOR_ADDR<<1,readbuff,2);
    uint16_t temp_reading = (((readbuff[0]<<8)|(readbuff[1]&0xFC)));
    uint8_t Temp_centigrade = (((175.72*temp_reading)/65536)-46.85);
    printf("Temperature := %d\n",Temp_centigrade);
}

// Calling this function Reads humidity
void humidity_sensor_read(void) {
    char rhread = HUMIDITY_READ;
    char readbuff[2];
    i2c.write(SENSOR_ADDR<<1,&rhread,1);
    i2c.read(SENSOR_ADDR<<1,readbuff,2);
    uint16_t Rh_reading = (((readbuff[0]<<8)|(readbuff[1]&0xFE)));
    uint8_t humidity_percentage = (((125*Rh_reading)/65536)-6);
    printf("Rhread := %d\n",humidity_percentage);
}/*
extern uint8_t sensordata_count_mcp0;
extern uint8_t sensordata_count_mcp1;
uint8_t sensor_buffer_MCP0[15];
uint8_t sensor_buffer_MCP1[15];
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
    
    printf("\n");*/
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
/*}

void channelwrite(int addr,char config)
{
    i2c.write(addr << 1, &config, 1);
}
void channelread(int addr,char config,uint8_t sensordata_count) {
    char readbuff[3];
  //  i2c.write(addr << 1, &config, 1);
    i2c.read(addr << 1, readbuff, 3);
    printf("gas :");
    if(sensordata_count == sensordata_count_mcp0) {
        for(int i=0; i < 2 ;i++) {
            sensor_buffer_MCP0[ i + (2 * sensordata_count)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf("\n");
    } else {
        for(int i=0; i < 2 ;i++) {
            sensor_buffer_MCP1[ i + (2 * sensordata_count)] = readbuff[i];    
            printf("%x ", readbuff[i]);
        }
        printf("\n");
    }
}*/
/*
void mcp23017_config(void)
{
  //  i2c.write(MCP23017_ADDR_GND << 1, mcp23_config_reg,2);
    
}
void soleniod_on()
{
    i2c.write(MCP23017_ADDR_VCC << 1, mcp23_IODIRA, 2);
  //  char config[3] = solenoid_on; 
    i2c.write(MCP23017_ADDR_VCC << 1, solenoid_reverse, 2);
}
void soleniod_off()
{
    i2c.write(MCP23017_ADDR_VCC << 1, mcp23_IODIRA, 2);
    i2c.write(MCP23017_ADDR_VCC << 1, solenoid_forward, 2); //reverse
}*/
