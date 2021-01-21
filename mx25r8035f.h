
#include "mbed.h"

uint8_t flash_write(uint8_t command);
uint8_t flash_read(uint8_t command);
void read_device_id(void);
void mx25r8035f_init();
void read_status_register(void);
void read_identification(void);
void read_config_register(void);
void flash_read(void);
void flash_write(void);
void flash_sector_erase(void);
void flash_wr_enable(void);
void flash_wr_disable(void);