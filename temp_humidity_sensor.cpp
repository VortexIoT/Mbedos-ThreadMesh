/*
  Oct 2020
*/
#include "temp_humidity_sensor.h"
#include <cstdint>

mbed::DigitalOut env_en(SENSE_ENABLE, 1);
mbed::I2C i2c(I2C_SDA,I2C_SCL);  //i2c init
/*#########################################  I2C Functions Body ###################***********/
// Call this function to set i2c frequency
void i2cinit(void) {
     i2c.frequency(100000);
}
  // Calling this function Reads indoor Rh %  and respective temperature
uint8_t *humidity_temp_read(void) {//Temp read with RH
    char rhread = RH_READ;
    char readbuff[2];
    uint8_t percentage_buffer[4];
    i2c.write(SENSOR_ADDR<<1,&rhread,1);
    i2c.read(SENSOR_ADDR<<1,readbuff,2);
    uint16_t Rh_reading = (((readbuff[0]<<8)|(readbuff[1]&0xFE)));
    uint16_t RH_percentage = (((125*Rh_reading)/65536)-6);
    percentage_buffer[0]= RH_percentage;
    percentage_buffer[1] = ' ';
    percentage_buffer[2] = ' ';
    printf("Rhread := %d\n",RH_percentage);
    rhread = TEMP_READ;
    i2c.write(SENSOR_ADDR<<1,&rhread,1);
    i2c.read(SENSOR_ADDR<<1,readbuff,2);
    uint16_t temp_reading = (((readbuff[0]<<8)|(readbuff[1]&0xFC)));
    uint16_t Temp_centigrade = (((175.72*temp_reading)/65536)-46.85);
    printf("Tempread := %d\n",Temp_centigrade);
    percentage_buffer[3] = Temp_centigrade;
    return percentage_buffer;
}
/*
 // Calling this function Reads indoor temperature
void temperature_read(void) //Seperate temp read
{
    char Temp = TEMPERATURE;
    char readbuff[2];
    i2c.write(SENSOR_ADDR<<1,&Temp,1);
    i2c.read(SENSOR_ADDR<<1,readbuff,2);

    uint16_t temp_reading = (((readbuff[0]<<8)|(readbuff[1]&0xFC)));
    uint16_t Temp_centigrade = (((175.72*temp_reading)/65536)-46.85);
    printf("Temperature := %d\n",Temp_centigrade);
}
*/