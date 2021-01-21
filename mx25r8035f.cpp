#include "mx25r8035f.h"

SPI spi(FLASH_MOSI, FLASH_MISO, FLASH_SCLK); // mosi, miso, sclk
DigitalOut cs(FLASH_CS);



void mx25r8035f_init() {
    spi.frequency(1000000); //i Mhz
    spi.format(8,0);//mode 0 and 8-bit format
}
/*

uint8_t flash_write(uint8_t command) {
    uint8_t res = 0;
    cs=0; //chi select low
    spi.write(command); //write enable
    cs = 1; //byte boundary
    return res;
}

uint8_t flash_read(uint8_t command) {
    uint8_t res = 0;
    cs=0; //chi select low
    res = spi.write(command); //WHOAMI reg for test purpose
    cs = 1; //deselect the chip
    return res;
}
void fun()
{
    int res[3] = {0};
     spi.write(0x06);
        spi.write(0x90); // t0 read manufacter id and device id 
    res[0] = spi.write(0x00); 
    res[1] = spi.write(0x00); 
    res[2] = spi.write(0x00);  //address byte to set the order of manufacturer and device out
    cs = 1; //deselect the chip
    printf ("manufacturerid : %x\n", res[0]);
    printf ("deviceid : %x %x\n", res[1],res[2]);

    cs=0; //chi select low
    spi.write(0x06); //write enable
    spi.write(0x9F); // t0 read manufacter id and device id 
    res[0] = spi.write(0x00); 
    res[1] = spi.write(0x00); 
    res[2] = spi.write(0x00); 
    cs = 1; //deselect the chip
    printf ("manufacturerid : %x\n", res[0]);
    printf ("deviceid : %x %x\n", res[1],res[2]);
}
*/
void read_device_id(void) {
   cs = 0;
   spi.write(0x90); //Manufacturer & device ID code
   spi.write(0x00); //dummy byte
   spi.write(0x00); //dummy byte
   spi.write(0x00); //address
   printf ("manufacturer-id : %x\n" ,spi.write(0x00));
   printf ("deviceid : %x", spi.write(0x00));
  // printf("%x\n", spi.write(0x00));
   cs= 1;
}
void read_identification(void) {
    cs = 0;
   spi.write(0x9F); //RDID code
   printf ("manufacturer-id : %x\n" ,spi.write(0x00));
   printf ("deviceid : %x", spi.write(0x00));
   printf("%x\n", spi.write(0x00));
   cs= 1;
}
void read_status_register(void) {
    cs = 0;
   spi.write(0x05); //Read status code
   printf ("status reg1 : %x\n" ,spi.write(0x00));
   printf ("status reg2 : %x\n" ,spi.write(0x00));
   cs = 1;
}

void read_config_register(void) {
    cs = 0;
   spi.write(0x15); //Read config code
   printf ("config reg1 : %x\n" ,spi.write(0x00));
   printf ("config reg2 : %x\n" ,spi.write(0x00));
   cs = 1;
}

void flash_read(void) {
   cs = 0;
   spi.write(0x03); //Read code
   spi.write(0x00); //address
   spi.write(0x10); //address
   spi.write(0x00); //address
   printf("data :");
   for(int i=0;i<256;i++) {
       printf("%x ", spi.write(0x00));
   }
   printf("\n");
   cs = 1;
}

void flash_write(void) {
    flash_wr_enable();
    cs = 0;
    spi.write(0x02); //pp code
    spi.write(0x00); //address
    spi.write(0x10); //address
    spi.write(0x14); //address
    for(int i = 0; i<256;i++)
    {
        spi.write(i);
    }
    cs = 1;
    flash_wr_disable();
}

void flash_sector_erase(void) {
    flash_wr_enable();
    cs = 0;
    spi.write(0x20); //pp code
    spi.write(0x00); //address
    spi.write(0x10); //address
    spi.write(0x00); //address
    cs = 1;
    flash_wr_disable();
}

void flash_wr_enable(void) {
    cs = 0;
    spi.write(0x06); //write enable command
    cs = 1;
}

void flash_wr_disable(void) {
    cs = 0;
    spi.write(0x04); //write disable
    cs = 1;
}