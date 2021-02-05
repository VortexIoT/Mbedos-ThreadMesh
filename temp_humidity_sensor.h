
#include "drivers/I2C.h"
#include "drivers/DigitalOut.h"

#define SENSOR_ADDR                 0x40
#define HUMIDITY_READ               0xE5
#define TEMP_READ                   0xE0
#define LSB_BYTE                    0xFC
#define TEMPERATURE_READ            0xE3
#define SENSOR_INTERVAL             60

//uint8_t Rh_Temp_sensor_cmdset[] = {0xE5,0xF5,0xE3,0xF3,0xE0,0xFE, 0xE6,0xE7,0x51,0x11,0xFA,0x0F,0xFC,0xC9,0x84,0xB8};
void i2cinit(void);
void humidity_temp_read(void);
void temperature_sensor_read(void);
void humidity_sensor_read(void);
void temp_hum_sensor_read_every_5min(void);
void stop_si7021_datacapture(void);
void temp_hum_sensor_read_every_5min(void);
void set_si7021_datacapture_intervel(int ms);