
#include "mbed.h"

static char fan_on[2] = {0x09,0xc0}; //logic high = 1, logic low = 0
static char fan_off[2] = {0x09,0x00}; //logic high = 1, logic low = 0
static char fan_gpio[2] = {0x12,0xc0}; //full on
static char fan_speed[2] = {0x00, 0x1f};
static char fan_fullon[2] = {0x02,0x04}; //full on
static char fan_fulloff[2] = {0x02,0x14}; //full on
static char fan_closedloop[2] = {0x02,0x24}; //full on
static char fan_openloo[2] = {0x02,0x34}; //full on
void fan_config(void);
void fan_control(char *fancontrol_value);