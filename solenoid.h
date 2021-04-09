#include "mbed.h"



#define MCP23017_ADDR_VCC   0x21 //A0 connected to vcc using this now
#define MCP23017_ADDR_GND   0x20 //A0 connected to GND
/*

//char mcp23_IODIRB[2] = {0x10, 0x00};
char mcp_OLATA[2] = {0x0A, 0x03}; //solenoid 1
char mcp_OLATA_solenoid2[2] = {0x0A, 0x0c};
char mcp_OLATA_fan[2] = {0x0A,0xc0};
//have only 2 solenoids so instead 0x3f
char solenoid_on[2] = {0x09,0x0f}; //logic high = 1, logic low = 0 //all 1's means brake so 
char solenoid_off[2] = {0x09,0x00}; //logic high = 1, logic low = 0


*/
static char mcp23_IODIRA[2] = {0x00, 0x00};
static char solenoid_leftvalve_open[2] = {0x12,0x08}; //logic high = 1, logic low = 0 //all 1's means brake so 0x28
static char solenoid_leftvalve_close[2] = {0x12,0x04}; //logic high = 1, logic low = 0 0x14

static char solenoid_rightvalve_open[2] = {0x12,0x20}; //logic high = 1, logic low = 0 //all 1's means brake so 0x28
static char solenoid_rightvalve_close[2] = {0x12,0x10}; //logic high = 1, logic low = 0 0x14


void soleniod_valve_control(char *valveposition);
void mcp23017_config(void);
void soleniod_on();
void soleniod_off();