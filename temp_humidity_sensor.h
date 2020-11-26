
#include "drivers/I2C.h"
#include "drivers/DigitalOut.h"

#define I2C_SDA                     PC4
#define I2C_SCL                     PC5
#define SENSE_ENABLE                PF9
#define SENSOR_ADDR                 0x40
#define RH_READ         0xE5
#define TEMP_READ       0xE0
#define LSB_BYTE        0xFC
#define TEMPERATURE     0xE3


//uint8_t Rh_Temp_sensor_cmdset[] = {0xE5,0xF5,0xE3,0xF3,0xE0,0xFE, 0xE6,0xE7,0x51,0x11,0xFA,0x0F,0xFC,0xC9,0x84,0xB8};
void i2cinit(void);
void humidity_temp_read(uint8_t *percentage_buffer);
void temperature_read(void);
