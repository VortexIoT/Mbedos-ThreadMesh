/*
 As mentioned in AQM breakout solenoid1 is not connected to PA0 and PA1
 its  connected to PA4 and PA5
 PA0, PA1 -> solenoid3 0x02, 0x01
 PA2, PA3 -> solenoid2 0x08, 0x04
 PA4, PA5 -> solenoid1  0x20, 0x10


*/


#include "solenoid.h"
#include "temp_humidity_sensor.h"

//extern mbed::I2C i2c;

void mcp23017_config(void) {
  //  i2c.write(MCP23017_ADDR_GND << 1, mcp23_config_reg,2);
    i2c.write(MCP23017_ADDR_VCC << 1, mcp23_IODIRA, 2);
}
void soleniod_valve_control(char *valveposition) {
    i2c.write(MCP23017_ADDR_VCC << 1, valveposition, 2);
}
/*
void soleniod_valve_close(char *valveposition) {
//   i2c.write(MCP23017_ADDR_VCC << 1, mcp23_IODIRA, 2);
    i2c.write(MCP23017_ADDR_VCC << 1, valveposition, 2); //reverse
}*/