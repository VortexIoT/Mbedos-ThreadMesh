
#include "mbed.h"
#include <cstdint>

uint8_t flash_write(uint8_t command);
uint8_t flash_read(uint8_t command);
void read_device_id(void);
void mx25r8035f_init();
uint8_t read_status_register(void);
void read_identification(void);
void read_config_register(void);
void flash_read(void);
void flash_pagewrite(void);
void flash_sector_erase(void);
void flash_wr_enable(void);
void flash_wr_disable(void);
void flash_stringwrite(uint8_t *str, int8_t pageaddr);
void test_fun(uint8_t *str);
void mx25r8035f_read(void);
void flash_page_rd(int8_t pageaddr);
void block_erase();

/*#####################
*/
void block64_erase(uint8_t blocknumber);
void mx25r8035f_write(uint8_t *inputfile);
void block32_erase(uint8_t blocknumber);
void flash_sector_erase(uint8_t sectornumber);
//void pages_read(uint16_t pageaddress);
//void pages_write(uint8_t *data, uint16_t pageaddress);
void pages_write(uint8_t *data, int bytes_towrite, uint16_t pageaddress);
void pages_read(uint16_t pageaddress,uint16_t readsize);

//page, sector, block32, block64 read and sector, block32, block64 erase

void flash_sector_erase(uint8_t sectornumber);
void block32_erase(uint8_t blocknumber);
void block64_erase(uint8_t blocknumber);
void Page_read(uint16_t pagenumber);
void block64_read(uint8_t blocknumber);
void block32_read(uint8_t blocknumber);
void sector_read(uint16_t sectornumber);
void event_handlerflash();
void chip_erase(void);
void coap_payload_write();
void coap_payload_read(uint16_t pagenumber, uint32_t payloadlength);
void coap_payload_write(uint16_t pagenumber);