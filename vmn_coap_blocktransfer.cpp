/*
  Block1 for request (post, put)
  Block2 for response (Get,post,put)

  consider block size as 2^(SZX+4) if size I require is 64 then SZX = 2
  M = any following blocks



*/
#include "mbed.h"
#include "vmn_coap_blocktransfer.h"

struct coap_s* coapblockhandle;//=sn_coap_protocol_init(&coap_client_malloc, &coap_client_free, &coap_client_tx_cb, &coap_client_rx_cb);
coap_version_e coapversion = COAP_VERSION_1;
void* coap_blocktransfer_malloc(uint16_t size) {
    return malloc(size);
}

void coap_blocktransfer_free(void* addr) {
    free(addr);
}
// tx_cb and rx_cb are not used in this program
uint8_t coap_blocktransfer_tx_cb( uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d ) {
    printf("coap tx cb\n");
    return 0;
}

int8_t coap_blocktransfer_rx_cb( sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c ) {
    printf("coap rx cb\n");
    return 0;
}
void blockl()
{
    coapblockhandle = sn_coap_protocol_init(&coap_blocktransfer_malloc, &coap_blocktransfer_free, &coap_blocktransfer_tx_cb, &coap_blocktransfer_rx_cb); //creating handler to access the packets
    sn_coap_protocol_set_block_size(coapblockhandle, 64); //block size 64
}


