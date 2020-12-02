#include "UDPSocket.h"
//#include "sn_nsdl.h"
#include "sn_coap_protocol.h"
#include "sn_coap_header.h"

#define IPADDRESS2    "fd00:db8:0:0:0:ff:fe00:2000"
#define IPADDRESS1    "fd00:db8:0:0:92cf:7276:ce01:3271" 
#define IPADDRESS     "fd00:db8:0:0:a58a:2805:667e:9cef"

extern uint8_t RH_percentage;
extern uint8_t Temp_centigrade;

//extern SocketAddress sockAddr;
extern MeshInterface *mesh;



int8_t coap_server_rx_cb(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c);
uint8_t coap_server_tx_cb(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d);
void coap_server_free(void* addr);
void* coap_server_malloc(uint16_t size);
//uint8_t Packet_builder(char *host_address, uint8_t *uri_path, uint8_t msg_code,uint8_t msg_type,uint8_t *payload, uint16_t payload_len);
//void Packet_parser(uint8_t ret,uint8_t *receive_buff_data,uint8_t method);
void coap_init(uint8_t msg_Code, uint8_t msg_type,uint8_t *payload, uint16_t payload_len);
uint8_t getfunction(void);
SocketAddress coap_init1(void);
SocketAddress coap_config(char *host_address);
//void handle_socket(void);
void receive_msg(void);
void coap_server_init(char *ipaddr) ;
void text(void);
void coapserver_response_build(SocketAddress addr,sn_coap_hdr_s* coap_res_ptr,uint8_t ack_pack_len);
void receive_msg(void);
void response_packet_formation(SocketAddress addr, sn_coap_hdr_s* coap_build_res_ptr, uint16_t response_messageid,char *payload);
//void coapserver_response_builder(void);