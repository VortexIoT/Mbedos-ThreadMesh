/*
  Oct 2020
*/
#include "temp_humidity_sensor.h"
#include <cstdint>
#include "mbed.h"

//mbed::DigitalOut env_en(SENSE_ENABLE, 1);
//uint8_t Temp_centigrade;
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
}
