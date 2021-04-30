
#include "mbed.h"
#include "stdio.h" 
#include "thread_management_if.h"
#include "protocol_abstract.h"

/*********************  Variable Decalrations START **************************/
/*  Network Parameters */
/*static uint8_t extpanid[8] = {0xf1, 0xb5, 0xa1, 0xb2,0xc4, 0xd5, 0xa1, 0xbd };
static uint8_t masterkey[16] ={0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};//{ 0x00, 0xeb, 0x64, 0x37, 0x02, 0x4c, 0x86, 0x8d, 0xdd, 0x2b, 0x18, 0xde, 0x62, 0xc7, 0x98, 0x68};
static uint16_t panid= 0x0700;
static uint8_t Network_name[16] = "Thread Network";
static uint16_t channel = 22;
static uint8_t meshprefix[8] = {0xfd, 0x0, 0x0d, 0xb8, 0x0, 0x0, 0x0, 0x0};
static uint8_t channel_mask[9] = "07fff800";
static uint8_t psk[16] = MBED_CONF_MBED_MESH_API_THREAD_CONFIG_PSKC;
static uint8_t securitypolicy = 255; 
static uint64_t local_timestamp = 0x10000;
static uint32_t timeinseconds = 0;
*/
/*********************  Variable Decalrations START **************************/
extern MeshInterface *mesh;
extern uint8_t getcmd_count;
//extern link_configuration_s *link;
extern link_configuration *linkcopy;
extern link_configuration *link;
extern uint8_t local_channel_page;/**< channel page supported pages 0*/
extern uint16_t local_key_rotation; /**< Key rotation time in hours*/
extern uint32_t local_key_sequence;
extern uint8_t nwparams_change_identification; //using for test purpose has to think again
extern uint8_t keeping_nw_default_details;


void thread_eui64_trace();
uint8_t mesh_connect(void);
void mesh_disconnect(void);
void read_ipaddr(void);
uint8_t values_to_set(char *networkparameter, char *value);
void datasetcommit_active(void);
link_configuration_s *linkconfiguration_copy(link_configuration_s *addlink, link_configuration_s *configuration_ptr);
link_configuration_s *link_configuration_create(void);