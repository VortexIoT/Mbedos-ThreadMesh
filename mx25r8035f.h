
#include "mbed.h"
#include <cstdint>

uint8_t flash_write(uint8_t command);
uint8_t flash_read(uint8_t command);
void read_device_id(void);
void mx25r8035f_init();
uint8_t mx25r8035f_get_status(void);
void mx25r8035f_get_device_id(void);
void mx25r8035f_get_config(void);
void flash_read(void);
void flash_pagewrite(void);
void flash_sector_erase(void);
void mx25r8035f_wr_enable(void);
void mx25r8035f_wr_disable(void);
void flash_stringwrite(uint8_t *str, int8_t pageaddr);
void test_fun(uint8_t *str);
void mx25r8035f_read(void);
void flash_page_rd(int8_t pageaddr);
void block_erase();

/*#####################
*/

void mx25r8035f_write(uint8_t *inputfile);


//void pages_read(uint16_t pageaddress);
//void pages_write(uint8_t *data, uint16_t pageaddress);
void mx25r8035f_pages_write(uint8_t *data, int bytes_towrite, uint16_t pageaddress);
void mx25r8035f_pages_read(uint16_t pageaddress,uint16_t readsize);

//page, sector, block32, block64 read and sector, block32, block64 erase

void mx25r8035f_sector_erase(uint8_t sectornumber);
void mx25r8035f_block32_erase(uint8_t blocknumber);
void mx25r8035f_block64_erase(uint8_t blocknumber);
void mx25r8035f_page_read(uint16_t pagenumber);
void mx25r8035f_block64_read(uint8_t blocknumber);
void mx25r8035f_block32_read(uint8_t blocknumber);
void mx25r8035f_sector_read(uint16_t sectornumber);
void event_handlerflash();
void mx25r8035f_chip_erase(void);
void coap_payload_write();
void coap_payload_read(uint16_t pagenumber, uint32_t payloadlength);
void coap_payload_write(uint16_t pagenumber);

int mx25r8035f_page_read(uint8_t *buffer, uint16_t pagecnt, uint16_t readsize);