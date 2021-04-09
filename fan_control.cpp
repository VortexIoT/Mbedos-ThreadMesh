
//Try to implemet tachometer

#include "solenoid.h"
#include "temp_humidity_sensor.h"
#include "fan_control.h"

#define MAX6650_FAN_CONTROLLER_ADD  0x1F    //0011 111
#define MAX6650_ADDR    0x1f
void fan_config(void) {
    i2c.write(MCP23017_ADDR_VCC << 1, mcp23_IODIRA, 2);
    i2c.write(MCP23017_ADDR_VCC << 1, fan_gpio, 2);
}
void fan_control(char *fancontrol_value) {
    i2c.write(MAX6650_ADDR << 1, fancontrol_value, 2);
}