
//#include "cstring"
#include "thread_management_if.h"
#include "InternetSocket.h"
#include "protocol_abstract.h"
#include "RawSerial.h"
#include "sn_coap_protocol.h"
#include "sn_coap_header.h"

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

/*-------------------  Function declarations END -----------------------*/