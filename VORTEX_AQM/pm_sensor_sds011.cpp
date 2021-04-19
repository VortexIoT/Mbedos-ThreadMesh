#include "pm_sensor_sds011.h"
#include "temp_humidity_sensor.h"
#include "solenoid.h"

//configures the pins as output
void pm_sensor_config(void) {
    i2c.write( MCP23017_ADDR << 1, mcp23_IODIRB, 2 ); 
}

//controls the state of the PM sensor
void pm_sensor_control(char *pm_sensor_onoff) {
 i2c.write( MCP23017_ADDR << 1, pm_sensor_onoff, 2);
}