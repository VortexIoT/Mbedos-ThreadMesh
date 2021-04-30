
//#include "cstring"
#include "thread_management_if.h"
#include "InternetSocket.h"
#include "protocol_abstract.h"
#include "RawSerial.h"
#include "sn_coap_protocol.h"
#include "sn_coap_header.h"

/*
static uint8_t local_channel_page = 0;//channel page supported pages 0
static uint16_t local_key_rotation = 3600; // Key rotation time in hours
static uint32_t local_key_sequence = 0;
static uint8_t nwparams_change_identification = 0; //using for test purpose has to think again
static uint8_t keeping_nw_default_details = 1;*/

/*  Network Parameters */
extern uint8_t extpanid[8];// = {0xf1, 0xb5, 0xa1, 0xb2,0xc4, 0xd5, 0xa1, 0xbd };
extern uint8_t masterkey[16];// ={0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};//{ 0x00, 0xeb, 0x64, 0x37, 0x02, 0x4c, 0x86, 0x8d, 0xdd, 0x2b, 0x18, 0xde, 0x62, 0xc7, 0x98, 0x68};
extern uint16_t panid;//= 0x0700;
extern uint8_t Network_name[16];// = "Thread Network";
extern uint16_t channel;// = 22;
extern uint8_t meshprefix[8];// = {0xfd, 0x0, 0x0d, 0xb8, 0x0, 0x0, 0x0, 0x0};
extern uint8_t channel_mask[9];// = "07fff800";
extern uint8_t psk[16];// = MBED_CONF_MBED_MESH_API_THREAD_CONFIG_PSKC;
extern uint8_t securitypolicy;// = 255; 
extern uint64_t local_timestamp;// = 0x10000;
extern uint32_t timeinseconds;// = 0;
/*-------------------  Function declarations START -----------------------*/

void isr_receive(void);
void hexfile_format(char ascii_formof_hexfile[]);
uint8_t chartohex(char ch);
void string_to_hex(char *str, uint8_t *output, uint8_t len);
uint8_t strtohex8(char str[2]);
uint16_t strtohex16(char str[4]);
int16_t hextoint(uint8_t hex);
uint8_t inttohex(uint8_t intvalue);
void dataset_commands_List(void);
void cli_cmds_Handler(char *str);
void scan_network_details(void);
//uint8_t values_to_set(char *str, char *value);
void get_values(char *networkparameter,uint8_t len);
uint16_t panid_value(char *str);
void get_keyword_cmds(char *networkparam,uint8_t param_len);
void dataset_keyword_cmds(char *networkparameter, char *value,uint8_t netwrkparam_len,uint8_t value_len);
void threadkeyword_cmds(char *cmd,uint8_t len);
void coap_request_cmds(char *str);
//void thread_eui64_trace();
//uint8_t mesh_connect();
//void mesh_disconnect(void);
//void read_ipaddr(void);
//void datasetcommit_active(void);

//not using functions
uint8_t stringcmp_fun(char *param);
void transmit_interrupt(char *Data_buff);
uint8_t stringtoken(char *str);
/*-------------------  Function declarations END -----------------------*/