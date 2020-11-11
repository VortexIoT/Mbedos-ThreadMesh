
//#include "cstring"
#include "thread_management_if.h"
#include "InternetSocket.h"
#include "protocol_abstract.h"
#include "RawSerial.h"
//#include "Serial.h"




/*-------------------  Function declarations START -----------------------*/
void ISR_Rx();
uint8_t Mesh_connect();
void Mesh_disconnect(void);
void dataset_commands_List(void);
void cli_cmds_Handler(char *str);
uint8_t stringtoken(char *str);
void Read_IPaddr(void);
void datasetcommit_active(void);
void Scan_Network_details(void);
uint8_t Values_to_set(char *str, char *value);
void Get_values(char *networkparameter,uint8_t len);
link_configuration_s *linkconfiguration_copy(link_configuration_s *addlink, link_configuration_s *configuration_ptr);
link_configuration_s *link_configuration_create(void);
void Transmit_interrupt(char *Data_buff);
uint8_t stringcmp_fun(char *param);
void get_keyword_cmds(char *networkparam,uint8_t param_len);
void dataset_keyword_cmds(char *networkparameter, char *value,uint8_t netwrkparam_len,uint8_t value_len);
void threadkeyword_cmds(char *cmd,uint8_t len);

/*-------------------  Function declarations END -----------------------*/