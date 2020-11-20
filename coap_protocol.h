#include "UDPSocket.h"
//#include "sn_nsdl.h"
#include "sn_coap_protocol.h"
#include "sn_coap_header.h"


int8_t coap_rx_cb(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c);
uint8_t coap_tx_cb(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d);
void coap_free(void* addr);
void* coap_malloc(uint16_t size);
uint8_t Packet_builder(char *host_address, uint8_t *uri_path, uint8_t msg_code,uint8_t msg_type,uint8_t *payload, uint16_t payload_len);
void Packet_parser(uint8_t ret,uint8_t *receive_buff_data,uint8_t method);
void coap_init(uint8_t msg_Code, uint8_t msg_type,uint8_t *payload, uint16_t payload_len);
uint8_t getfunction(void);
SocketAddress coap_init1(void);
SocketAddress coap_config(char *host_address);
void handle_socket(void);
void receive_msg(void);
void coap_server_init(void) ;
void text(void);
void coapserver_response_builder(void);
//void coapserver_response_builder(void);