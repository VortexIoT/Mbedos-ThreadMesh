
#include "mbed.h"
#include <cstdint>


void read_device_id(void);
void mx25r8035f_init();
uint8_t mx25r8035f_get_status(void);
void mx25r8035f_get_device_id(void);
void mx25r8035f_get_config(void);
void mx25r8035f_wr_enable(void);
void mx25r8035f_wr_disable(void);
void mx25r8035f_read(void);
void mx25r8035f_write(char *inputfile);
void mx25r8035f_singlepage_write(char *data, int bytes_towrite, uint16_t pageaddress);
void mx25r8035f_pages_read(uint16_t pageaddress,uint16_t readsize);
void mx25r8035f_singlepage_read(uint16_t pagenumber);
void mx25r8035f_block64_read(uint8_t blocknumber);
void mx25r8035f_block32_read(uint8_t blocknumber);
void mx25r8035f_sector_read(uint16_t sectornumber);
//page, sector, block32, block64 read and sector, block32, block64 erase
void mx25r8035f_sector_erase(uint8_t sectornumber);
void mx25r8035f_block32_erase(uint8_t blocknumber);
void mx25r8035f_block64_erase(uint8_t blocknumber);
void mx25r8035f_chip_erase(void);
void event_handlerflash();
void coap_payload_write();
void coap_payload_read(uint16_t pagenumber, uint32_t payloadlength);
void coap_payload_write(uint16_t pagenumber);
int mx25r8035f_page_read(uint8_t *buffer, uint16_t pagecnt, uint16_t readsize);