
//#include "cstring"
#include "thread_management_if.h"
#include "InternetSocket.h"
#include "protocol_abstract.h"
#include "RawSerial.h"
#include "sn_coap_protocol.h"
#include "sn_coap_header.h"


/*********************  Variable Decalrations START **************************/
/*  Network Parameters */
static uint8_t extpanid[8] = {0xf1, 0xb5, 0xa1, 0xb2,0xc4, 0xd5, 0xa1, 0xbd };
static uint8_t masterkey[16] ={0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};//{ 0x00, 0xeb, 0x64, 0x37, 0x02, 0x4c, 0x86, 0x8d, 0xdd, 0x2b, 0x18, 0xde, 0x62, 0xc7, 0x98, 0x68};
static uint16_t panid= 0x0700;;
static uint8_t Network_name[16] = "Thread Network";
static uint16_t channel = 22;
static uint8_t meshprefix[8] = {0xfd, 0x0, 0x0d, 0xb8, 0x0, 0x0, 0x0, 0x0};
static uint8_t channel_mask[9] = "07fff800";
static uint8_t psk[16] = MBED_CONF_MBED_MESH_API_THREAD_CONFIG_PSKC;
static uint8_t securitypolicy = 255; 
static uint64_t local_timestamp = 0x10000;

/*-------------------  Function declarations START -----------------------*/

void isr_rx();
uint8_t mesh_connect();
void mesh_disconnect(void);
void dataset_commands_List(void);
void cli_cmds_Handler(char *str);
uint8_t stringtoken(char *str);
void read_ipaddr(void);
void datasetcommit_active(void);
void scan_network_details(void);
uint8_t values_to_set(char *str, char *value);
void get_values(char *networkparameter,uint8_t len);
link_configuration_s *linkconfiguration_copy(link_configuration_s *addlink, link_configuration_s *configuration_ptr);
link_configuration_s *link_configuration_create(void);
void transmit_interrupt(char *Data_buff);
uint8_t stringcmp_fun(char *param);
void get_keyword_cmds(char *networkparam,uint8_t param_len);
void dataset_keyword_cmds(char *networkparameter, char *value,uint8_t netwrkparam_len,uint8_t value_len);
void threadkeyword_cmds(char *cmd,uint8_t len);
void coap_request_cmds(char *str);
void thread_eui64_trace();
uint8_t chartohex(char ch);
void string_to_hex(char *str, uint8_t *output, uint8_t len);
void hexfile_format(char ascii_formof_hexfile[]);

uint8_t strtohex8(char str[2]);
uint16_t strtohex16(char str[4]);
int16_t hextoint(uint8_t hex);
uint8_t inttohex(uint8_t intvalue);
/*-------------------  Function declarations END -----------------------*/