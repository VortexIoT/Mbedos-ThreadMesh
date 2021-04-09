#include "pm_sensor_sds011.h"
#include "temp_humidity_sensor.h"
#include "solenoid.h"

void aqm_config(void) {
    i2c.write(MCP23017_ADDR_VCC<<1, mcp23_IODIRB,2);
}
void aqm_control(char *aqm_control_onoff) {
 i2c.write(MCP23017_ADDR_VCC<<1, aqm_control_onoff,2);
}