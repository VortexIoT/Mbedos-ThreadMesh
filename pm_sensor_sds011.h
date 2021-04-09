
static char mcp23_IODIRB[2] = {0x01, 0x00}; //gpios as output to active pm sensor
static char aqm0_on[2] = {0x13,0x01}; //logic high = 1, logic low = 0 //all 1's means brake so 0x28
static char aqm0_off[2] = {0x13,0x00}; //logic high = 1, logic low = 0 //all 1's means brake so 0x28
static char aqm1_on[2] = {0x13,0x02};
static char aqm1_off[2] = {0x13,0x00};

void aqm_config(void);
void aqm_control(char *aqm_control_onoff);