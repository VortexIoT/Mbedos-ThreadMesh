#include "mbed.h"


#define MCP3423_0_ADDR 0x68
#define MCP3423_1_ADDR 0x6C
//one-shot, 12bit sample rate x1 gain
#define MCP3423_CH0_CONFIG_G1    0x80
#define MCP3423_CH0_CONFIG_G2    0x81
#define MCP3423_CH0_CONFIG_G4    0x82
#define MCP3423_CH0_CONFIG_G8    0x83
#define MCP3423_CH1_CONFIG_G1    0xA0
#define MCP3423_CH1_CONFIG_G2    0xA1
#define MCP3423_CH1_CONFIG_G4    0xA2
#define MCP3423_CH1_CONFIG_G8    0xA3

static uint8_t sensordata_count_mcp0 = 0;
static uint8_t sensordata_count_mcp1 = 1;
static uint8_t sensordata_count_mcp2 = 2;
static uint8_t sensordata_count_mcp3 = 3;
static uint8_t sensor_buffer_MCP0[15];
static uint8_t sensor_buffer_MCP1[15];
static uint8_t sensor_buffer_MCP2[15];
static uint8_t sensor_buffer_MCP3[15];

void adcchannelwrite(int addr,char config);
void adcchannelread(int addr,char config, uint8_t sensordata_count);