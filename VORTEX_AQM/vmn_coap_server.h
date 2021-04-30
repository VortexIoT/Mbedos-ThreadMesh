#include "UDPSocket.h"
//#include "sn_nsdl.h"
#include "sn_coap_protocol.h"
#include "sn_coap_header.h"
#include <cstdint>

#define IPADDRESS2    "fd00:db8:0:0:0:ff:fe00:2000"
#define IPADDRESS1    "fd00:db8:0:0:92cf:7276:ce01:3271" 
#define IPADDRESS     "fd00:db8:0:0:a58a:2805:667e:9cef"
//#define IPADDRESS2    "fd00:db8:0:0:0:ff:fe00:2000"
//#define IPADDRESS1    "fd00:db8:0:0:2e5e:ccdc:af4:87e3"//"fd00:db8:0:0:92cf:7276:ce01:3271" dongle cli
//#define IPADDRESS     "fd00:db8:0:0:a58a:2805:667e:9cef" //gateway

#define MAX_COAP_PAYLOAD_SIZE  65535
#define HEXSYNC1  56
#define HEXSYNC2  0xd7
#define ASCIISYNC1 'V'
#define DECSYNC1  86
#define DECSYNC2  215

/*
extern uint8_t masterkey[16];
extern uint16_t panid;
extern uint8_t extpanid[8];
extern uint8_t Network_name[16];
extern uint8_t meshprefix[8];
extern uint16_t channel;*/
//extern SocketAddress sockAddr;
extern MeshInterface *mesh;

int8_t coap_server_rx_cb(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c);
uint8_t coap_server_tx_cb(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d);
void coap_server_free(void* addr);
void* coap_server_malloc(uint16_t size);
void receive_msg(void);
void coap_server_init(void);//char *ipaddr) ;
void coapserver_response_build( SocketAddress addr,sn_coap_hdr_s* coap_res_ptr );
void send_response_packet(SocketAddress addr, sn_coap_hdr_s* coap_build_res_ptr);//, uint16_t response_messageid,char *payload);
void requested_message_code(uint8_t msg_code);
void requested_message_type(uint8_t msg_type);
void ack_vmn_coapmsg_build(uint8_t actualdata);
void nonack_vmn_coapmsg_build(uint8_t actualdata);
//void nonack_vmn_coapmsg_build(uint8_t actualdata, uint8_t *vmn_coapmsg);

void ack_nonack_generic_msg(uint16_t messageid,uint16_t payloadlength, uint8_t *ack_nonack_payload);
void operational_commands(uint16_t messageid);
//void configuration_commands(uint16_t messageid);
void configuration_commands(uint16_t messageid,uint16_t recvpayloadlen, uint8_t recvpayload[]);
void sensordata_measure_commands(uint16_t messageid);
void devicedata_monitor_commands(uint16_t messageid);
void errorcode_commands(uint16_t messageid);
void firmwareupdate_commands(uint16_t messageid);
void vmn_dpd_extract(void) ;
void nonack_frm_server_to_client(void);
void ack_frm_server_to_client(void);
void ack_si7021_response(uint8_t actualdata);
void nonack_si7021_response(uint8_t actualdata);