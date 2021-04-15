#include "vmn_coap_server.h"
#include "stdlib.h"
#include "stdio.h"
#include "mbed.h"
#include <cstdint>
/*
extern uint8_t masterkey[16];
extern uint16_t panid;
extern uint8_t extpanid[8];
extern uint8_t Network_name[16];
extern uint8_t meshprefix[8];
extern uint16_t channel;
*/
static uint16_t coap_msg_payload_len;
static uint8_t coap_msg_payload[200];

void* coap_client_malloc(uint16_t size);
void coap_client_free(void* addr);
uint8_t coap_client_tx_cb(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d);
int8_t coap_client_rx_cb(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c);
void client_requestpacket_build(char *host_address, uint8_t *uri_path, uint8_t msg_code,uint8_t msg_type,uint8_t *payload, uint16_t payload_len);
void client_responsepacket_parse(SocketAddress addr, uint8_t method);
void response_message_code(uint8_t msg_code);


