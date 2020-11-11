/*
  Oct 2020
*/


#include "temp_humidity_sensor.h"


mbed::DigitalOut env_en(SENSE_ENABLE, 1);
mbed::I2C i2c(I2C_SDA,I2C_SCL);  //i2c init

/*#########################################  I2C Functions Body ###################***********/

// Call this function to set i2c frequency
 void I2cinit(void)
 {
     i2c.frequency(100000);
 }
  // Calling this function Reads indoor Rh %  and respective temperature
void Humidity_Temp_Read(void) //Temp read with RH
{
    char rhread = RH_READ;
    char readbuff[2];
    i2c.write(SENSOR_ADDR<<1,&rhread,1);
    i2c.read(SENSOR_ADDR<<1,readbuff,2);

    uint16_t Rh_reading = (((readbuff[0]<<8)|(readbuff[1]&0xFE)));
    uint16_t RH_percentage = (((125*Rh_reading)/65536)-6);
    printf("Rhread := %d\n",RH_percentage);

    rhread = TEMP_READ;
    i2c.write(SENSOR_ADDR<<1,&rhread,1);
    i2c.read(SENSOR_ADDR<<1,readbuff,2);

    uint16_t temp_reading = (((readbuff[0]<<8)|(readbuff[1]&0xFC)));
    uint16_t Temp_centigrade = (((175.72*temp_reading)/65536)-46.85);
    printf("Tempread := %d\n",Temp_centigrade);
}

/*
 // Calling this function Reads indoor temperature
void Temperature_Read(void) //Seperate temp read
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