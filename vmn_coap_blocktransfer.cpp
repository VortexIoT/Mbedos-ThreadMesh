/*
  Block1 for request (post, put)
  Block2 for response (Get,post,put)

  consider block size as 2^(SZX+4) if size I require is 64 then SZX = 2
  M = any following blocks



*/

/*
#include "mbed.h"
void* coap_client_malloc(uint16_t size) {
    return malloc(size);
}

void coap_client_free(void* addr) {
    free(addr);
}
// tx_cb and rx_cb are not used in this program
uint8_t coap_client_tx_cb( uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d ) {
    printf("coap tx cb\n");
    return 0;
}

int8_t coap_client_rx_cb( sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c ) {
    printf("coap rx cb\n");
    return 0;
}
void blockl()
{

    sn_coap_protocol_set_block_size()
}
*/