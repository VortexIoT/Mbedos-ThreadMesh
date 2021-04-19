#include "mbed.h"
#include "UDPSocket.h"
//#include "sn_nsdl.h"
#include "sn_coap_protocol.h"
#include "sn_coap_header.h"

void* coap_blocktransfer_malloc(uint16_t size);
void coap_blocktransfer_free(void* addr);
uint8_t coap_blocktransfer_tx_cb( uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d );
int8_t coap_blocktransfer_rx_cb( sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c );