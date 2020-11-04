
#include "cstring"
#include "thread_management_if.h"
#include "InternetSocket.h"
#include "protocol_abstract.h"
#include "RawSerial.h"
//#include "Serial.h"




/*-------------------  Function declarations START -----------------------*/
void ISR_Rx();
uint8_t mesh_connection();
void Commands_List(void);
void Clicmd_format_making(char *str);
uint8_t stringtoken(char *str);
void Read_IPaddr(void);
void datasetcommit_active(void);
void Scan_Network_details(void);
uint8_t Values_to_set(char *str, char *value);
void Get_values(char *param);
link_configuration_s *linkconfiguration_copy(link_configuration_s *addlink, link_configuration_s *configuration_ptr);
link_configuration_s *link_configuration_create(void);
void Transmit_interrupt(char *Data_buff);
void i2cinit(void);
void Sensor_RH_Read(void);
void Sensor_Temp_Read(void);

/*-------------------  Function declarations END -----------------------*/

void init(int x);
