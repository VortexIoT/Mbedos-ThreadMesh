/*

Date 26/01/2021
hex file format
First character (:)  =  Start of a record                                     
Next two characters  =  Record length (in this example, 10h)                  
Next four characters =  Load address (in this example, 0080h)                 
Next two characters  =  Record type (see below)                               
Remaining characters =  Actual data                                           
Last two characters  =  Checksum (i.e., sum of all bytes + checksum = 00)    

*/


#include "mx25r8035f.h"
#include "string.h"
#include <algorithm>
#include <cstdint>
SPI spi(FLASH_MOSI, FLASH_MISO, FLASH_SCLK); // mosi, miso, sclk
DigitalOut cs(FLASH_CS);
//extern volatile char Rx_buff[1024 * 175];

uint8_t current_bytecount_lessthan_pagesize = 0;
int16_t previous_bytecount_lessthan_pagesize = 0;
int8_t bytecount_lessthan_pagesize_flag = 0;
uint16_t noof_pages_towrite_frmthisblock = 0;
uint16_t total_written_flashpages_count = 0;
uint16_t previous_written_pages_count = 0; 
int8_t block_entry=0;
extern char hexfile_buff[256];
EventQueue externalflash_rdwr_eventqueue;
uint8_t erase=0;

extern uint8_t *coap_payload;
void mx25r8035f_init() {
    spi.frequency(100000); //i Mhz
    spi.format(8,0);//mode 0 and 8-bit format
}
uint8_t flashhandle;
uint32_t pageaddress1=0;
void event_handlerflash(void) {

        flashhandle = externalflash_rdwr_eventqueue.call(mx25r8035f_pages_write,(uint8_t *)hexfile_buff, 16 ,pageaddress1++);
        externalflash_rdwr_eventqueue.cancel(flashhandle);    
}

/*------------------------ Device information --------------------------- */

void mx25r8035f_get_device_id(void) {
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
/*
void mx25r8035f_get_identification(void) {
    cs = 0;
   spi.write(0x9F); //RDID code
   printf ("manufacturer-id : %x\n" ,spi.write(0x00));
   printf ("device-id : %x", spi.write(0x00));
   printf("%x\n", spi.write(0x00));
   cs= 1;
}*/
uint8_t mx25r8035f_get_status(void) {
    uint8_t status_reg = 0;
    cs = 0;
    spi.write(0x05); //Read status code
    status_reg = spi.write(0x00);
    //printf ("status reg1 : %x\n" ,status_reg);//spi.write(0x00));
    //printf ("status reg2 : %x\n" ,spi.write(0x00));
    cs = 1;
    return status_reg;
}

void mx25r8035f_get_config(void) {
    cs = 0;
   spi.write(0x15); //Read config code
   printf ("config reg1 : %x\n" ,spi.write(0x00));
   printf ("config reg2 : %x\n" ,spi.write(0x00));
   cs = 1;
}
/*----------------------------------------------------------------------------*/

void mx25r8035f_wr_enable(void) {
    cs = 0;
    spi.write(0x06); //write enable command
    cs = 1;
}

void mx25r8035f_wr_disable(void) {
    cs = 0;
    spi.write(0x04); //write disable
    cs = 1;
}
/* commented on 18/02/21 to reduce the .hex size while testing bootloader uncomment later
void flash_pagewrite(void) {
    flash_wr_enable();
    cs = 0;

    //page address starts from 000000 to 0fffff hex total 4095 pages in 1MB
    //001000 -secto1, 020000-sector 2 ---1f000 -sector 31, 02f000 sector 47

    spi.write(0x02); //page write code
    spi.write(0x02); // sector address
    spi.write(0x04); //page 16
    spi.write(0x00); // start address 20 in page 16 address
    //write in page 16 start address from location 20 in that page  of sector 0
    for(int i = 0; i<256;i++)
    {
        spi.write('a');
    }
    cs = 1;
    flash_wr_disable();
}

*/

int mx25r8035f_page_read(uint8_t *buffer, uint16_t pagecnt, uint16_t readsize)  //this now testing per a block check it later for blocks
{
    int bytecnt=0;
    cs = 0;
    //page address starts from 000000 to 0fffff hex total 4095 pages in 1MB
    //001000 -secto1, 020000-sector 2 ---1f000 -sector 31, 02f000 sector 47
    spi.write(0x03); //page read code
    spi.write(((pagecnt & 0xff00) >> 8)); // sector address (pageaddress & 0xff00) >> 8
    spi.write((pagecnt & 0xff)); //page 16 (pageaddress & 0xff)
    spi.write(0x00); // start address 20 in page 16 address
    //write in page 16 start address from location 20 in that page  of sector 0
  //  for(uint8_t cnt = 0; cnt< pagecnt;cnt++) {
        for ( int i = 0; i < 2048; i++ ) {
            buffer[bytecnt++] = spi.write(0x00);
         //   printf("%02x ", buffer[bytecnt++]);
        }
 //  }
    cs = 1;
    return bytecnt;
}

void mx25r8035f_pages_read(uint16_t pageaddress, uint16_t readsize)  //this now testing per a block check it later for blocks
{
    int addr1, addr2;
    cs = 0;
    //page address starts from 000000 to 0fffff hex total 4095 pages in 1MB
    //001000 -secto1, 020000-sector 2 ---1f000 -sector 31, 02f000 sector 47
    spi.write(0x03); //page read code
    spi.write(((pageaddress & 0xff00) >> 8)); // sector address (pageaddress & 0xff00) >> 8
    spi.write((pageaddress & 0xff)); //page 16 (pageaddress & 0xff)
    spi.write(0x00); // start address 20 in page 16 address
    //write in page 16 start address from location 20 in that page  of sector 0
    for ( int i = 0; i < readsize; i++ ) {
        printf("%02x ", spi.write(0x00));
    }
    //  printf("\n");
    printf("page:%d\n",pageaddress);
    cs = 1;
}
/* commented on 18/02/21 to reduce the .hex size while testing bootloader uncomment later
void mx25r8035f_sector_erase(uint8_t sectornumber) {
    int64_t address = sectornumber * 4 * 1024;
    flash_wr_enable();
    cs = 0;
    spi.write(0x20); //pp code
    spi.write((address & 0xff0000) >> 16); //address
    spi.write((address & 0xff00) >> 8); //here erasing sector 1 i.e 01000 to 01fff hex is the sector address sector 0 starts from 00000 to 00fffh
    spi.write(address & 0xff); //address
    cs = 1;
    flash_wr_disable();
    erase = 1;
}

void mx25r8035f_block32_erase(uint8_t blocknumber) {
    int64_t address = blocknumber * 32 * 1024;
    flash_wr_enable();
    cs = 0;
    spi.write(0x52);
    spi.write((address & 0xff0000) >> 16);
    spi.write((address & 0xff00) >> 8);
    spi.write(address & 0xff);
    cs= 1;
    flash_wr_disable();
}
*/
void mx25r8035f_block64_erase(uint8_t blocknumber) {
    uint64_t address = blocknumber * 64 * 1024;
    mx25r8035f_wr_enable();
    cs = 0;
    spi.write(0xD8);
    spi.write((address & 0xff0000) >> 16);
    spi.write((address & 0xff00) >> 8);
    spi.write(address & 0xff);
    cs= 1;
 /*   if(total_written_flashpages_count > 256) {
     total_written_flashpages_count -=256;
     previous_bytecount_lessthan_pagesize = 0;
}
    else {
     total_written_flashpages_count = 0;
     previous_bytecount_lessthan_pagesize = 0;
     }
     erase = 1;*/
    mx25r8035f_wr_disable();
}
void mx25r8035f_chip_erase(void) {
    mx25r8035f_wr_enable();
    cs = 0;
    spi.write(0xc7);
    cs = 1;
    mx25r8035f_wr_disable();
}

/* commented on 18/02/21 to reduce the .hex size while testing bootloader uncomment later
void mx25r8035f_page_read(uint16_t pagenumber) {
    spi.lock();
    cs = 0;
    spi.write(0x03); //Read code
    spi.write((pagenumber & 0xff00) >> 8); //address
    spi.write((pagenumber & 0xff)); //page number 16
    spi.write(0x00); //page start read address 14 to  end 256
    printf("data :");
    for(int i=0;i < 256;i++) {
       printf("%c", spi.write(0x00));
    }
    printf("\n");
    cs = 1;
    spi.unlock();
}
void mx25r8035f_block64_read(uint8_t blocknumber) {
    uint64_t address = blocknumber * 64 * 1024;
    spi.lock();
    cs = 0;
    spi.write(0x03); //page read code
    spi.write((address & 0xff00) >> 8); // sector address (pageaddress & 0xff00) >> 8
    spi.write((address & 0xff)); //page 16 (pageaddress & 0xff)
    spi.write(0x00);
    printf("Block64 read\n");
    for(int j=0;j< 256 ; j++ ) {
        for(int i=0; i< 256;i++) {
            if( (j == 0) || (j==5) || (j == 10) || (j == 256)|| (j == 15))
                printf("%d", spi.write(0x00));
        }
        printf("\n");
    }
    cs = 1;
    spi.unlock();
}

void mx25r8035f_block32_read(uint8_t blocknumber) {
    uint64_t address = blocknumber * 32 * 1024;
    spi.lock();
    cs = 0;
    spi.write(0x03); //page read code
    spi.write((address & 0xff00) >> 8); // sector address (pageaddress & 0xff00) >> 8
    spi.write((address & 0xff)); //page 16 (pageaddress & 0xff)
    spi.write(0x00);
    printf("Block32 read\n");
    for(int i= 500; i< 32768;i++) {
        printf("%d", spi.write(0x00));
    }
    printf("\n");
    cs = 1;
    spi.unlock();
}
void mx25r8035f_sector_read(uint16_t sectornumber) {
    uint64_t address = sectornumber * 64 * 1024;
    spi.lock();
    cs = 0;
    spi.write(0x03); //page read code
    spi.write((address & 0xff00) >> 8); // sector address (pageaddress & 0xff00) >> 8
    spi.write((address & 0xff)); //page 16 (pageaddress & 0xff)
    spi.write(0x00);
    printf("sector read\n");
    for(int i = 0; i< 4096;i++) {
        printf("%d", spi.write(0x00));
    }
    cs = 1;
    spi.unlock();
    printf("\n");
}
*/

void mx25r8035f_pages_write(uint8_t *data, int bytes_towrite, uint16_t pageaddress)  //this now testing per a block check it later for blocks
{
 int addr = 0;
 //uint8_t *local_buff;
 /*   if(bytes_towrite == 256)
        addr = 0; 
    else {
        addr += bytes_towrite;
        if(addr >= 254)
         pageaddress +=1;
     //   strcat((char *)local_buff,(char *)data);
    }*/

//if( addr == 256)
{
  //  pageaddress++;
 
  /*  if( (bytecount_lessthan_pagesize_flag) && (block_entry == 2))
    {
        pageaddress +=1;
    }*/
  //  pageaddress += 1;
  mx25r8035f_wr_enable();
    spi.lock();
    cs = 0;

    //page address starts from 000000 to 0fffff hex total 4095 pages in 1MB
    //001000 -secto1, 020000-sector 2 ---1f000 -sector 31, 02f000 sector 47

    spi.write(0x02); //page write code
    spi.write(((pageaddress  & 0xff00) >> 8));//(pageaddress & 0xff00) >> 8);//(pageaddress & 0xff00) >> 8); // sector address 0x7f
    spi.write((pageaddress & 0xff)); //page 16 pageaddress & 0xff
    spi.write(0x00); // start address 20 in page 16 address
 //   if(bytes_towrite == 0)
 //       bytes_towrite = 256; //to write complete page
    //write in page 16 start address from location 20 in that page  of sector 0
 /*   if((read_status_register() & 0x01) == 0x01) 
    {
                wait_us(5000);
    for(int i = 0; i < bytes_towrite; i++) {
        spi.write(*data++);
     //   printf("%02x ",*data++);
    }
    }
    else {*/
     for(int i = 0; i < bytes_towrite; i++) {
        spi.write(*data++);
     //   printf("%02x",*data++);
  //  }
    }
  //  printf(" :addr %d\n",pageaddress);
    cs = 1;
    spi.unlock();
    mx25r8035f_wr_disable();
    printf("done\n");
}
    
}

void mx25r8035f_write(uint8_t *inputfile) //data received from terminal
{
    uint32_t file_length = strlen((char *)inputfile);
    int counter = 0;
    uint16_t current_page_position = 0;
    if(!previous_bytecount_lessthan_pagesize)
    {
        current_bytecount_lessthan_pagesize = file_length % 256;
        noof_pages_towrite_frmthisblock = file_length / 256; //to check no.of pages
     //   printf(" pages : %d mod : %d\n", noof_pages_towrite_frmthisblock, current_bytecount_lessthan_pagesize);
    }
    else {
        int diff = 256- previous_bytecount_lessthan_pagesize;
        file_length -= diff;
        current_bytecount_lessthan_pagesize = file_length % 256;
        noof_pages_towrite_frmthisblock = file_length / 256; //to check no.of pages
    //    printf(" pages : %d mod : %d\n", noof_pages_towrite_frmthisblock, current_bytecount_lessthan_pagesize);
        mx25r8035f_pages_write(inputfile, diff, total_written_flashpages_count);
        inputfile += diff;
     //   printf("leftover %d\n",diff);
        total_written_flashpages_count +=1;
    }
    
 //   block_entry += 1; //check whether it is first entry or not


 /*   if(previous_bytecount_lessthan_pagesize) //means second transfer
    {
    //  block_entry = 1; //clear the blocks entry flag
        pages_write(inputfile, 256 - previous_bytecount_lessthan_pagesize, total_written_flashpages_count);
        inputfile = inputfile-(256 - previous_bytecount_lessthan_pagesize);
        printf("leftover %d\n",256 - previous_bytecount_lessthan_pagesize);
        if(previous_bytecount_lessthan_pagesize == current_bytecount_lessthan_pagesize)
            current_bytecount_lessthan_pagesize = 0;
        total_written_flashpages_count +=1;
     //   previous_bytecount_lessthan_pagesize = 256;
        
    }
*/
//    if(current_bytecount_lessthan_pagesize)
 //       bytecount_lessthan_pagesize_flag = 1;
  //   if(noof_pages_towrite_frmthisblock == 0) { //size is less than or euql to 256 
  //      pages_write(inputfile, 0x00); //firstpage
  //  }
  //  else { //if size is more than 256bytes
        while(counter < noof_pages_towrite_frmthisblock) {
            current_page_position = total_written_flashpages_count + counter;
            if((mx25r8035f_get_status() & 0x01) == 0x01) {
            //    wait_us(5000);
                mx25r8035f_pages_write( inputfile+(256 * counter), 256, current_page_position);//counter );
            }
            else
                mx25r8035f_pages_write( inputfile+(256 * counter), 256, current_page_position);//counter );
            counter++;
        }
        if(current_bytecount_lessthan_pagesize)
        {
        //    wait_us(5000);
            mx25r8035f_pages_write( inputfile+(256 * (counter)), current_bytecount_lessthan_pagesize, current_page_position + 1);
        }
            
 //   }
    previous_bytecount_lessthan_pagesize = current_bytecount_lessthan_pagesize;
 //   if(current_bytecount_lessthan_pagesize)
    total_written_flashpages_count += noof_pages_towrite_frmthisblock;
  //  previous_written_pages_count += noof_pages_towrite_frmthisblock;
  //  printf("totalpages : %d\n",total_written_flashpages_count);
}
extern uint16_t written_pages_count;
void mx25r8035f_read(void) { //commented 09/03/21 test purpose
    int counter=0;

    printf("data :"); 
 
    //first page reading has problem look for this problem
        while (counter < written_pages_count) {//total_written_flashpages_count) {//noof_pages_towrite_frmthisblock) {
            if((mx25r8035f_get_status() & 0x01) == 0x01) {
                wait_us(5000);
                mx25r8035f_pages_read(counter, 256);
            } else
                mx25r8035f_pages_read(counter, 256); 
                counter++;
        }
   //     written_pages_count = 0;
    //    printf("pvs count %d\n", previous_bytecount_lessthan_pagesize);
    //    if(previous_bytecount_lessthan_pagesize)
    //        mx25r8035f_pages_read(counter, previous_bytecount_lessthan_pagesize);
 //   }
}
/* commented on 18/02/21 to reduce the .hex size while testing bootloader uncomment later
void mx25r8035f__write256(uint8_t *data)
{
    uint8_t datalen = strlen ((char *)data);
    current_bytecount_lessthan_pagesize = datalen % 256;
    mx25r8035f_pages_write( data, current_bytecount_lessthan_pagesize, 0);
}*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*commented on 18/02/21 to reduce the .hex size while testing bootloader uncomment later
void coap_payload_write(uint16_t pagenumber) { 
    mx25r8035f_pages_write(coap_payload, strlen((char *)coap_payload), pagenumber);
}
void coap_payload_read(uint16_t pagenumber, uint32_t payloadlength) {
    spi.lock();
    cs = 0;
    spi.write(0x03); //Read code
    spi.write((pagenumber & 0xff00) >> 8); //address
    spi.write(pagenumber & 0xff); //address
    spi.write(0x00); //address
    for(int i = 0; i < payloadlength; i++) {
       printf("%c", spi.write(0x00));
    }
    printf("\n");
    cs = 1;
    spi.unlock();
}

void beforemodifiedpages_write(uint8_t *data, uint16_t pageaddress)  //this now testing per a block check it later for blocks
{
    int addr1, addr2;
  //  if(pageaddress <= 127)
 //   {
  //      addr1 = 0x00;
   //     addr2 = pageaddress & 0xff;
        
        
   // }
   // else {
    //    addr1 = 0X00;// pageaddress / 127;
    //    addr2 = (pageaddress - 128) & 0xff;
    //}

    if( (bytecount_lessthan_pagesize_flag) && (block_entry == 2))
    {
        pageaddress +=1;
    }
    flash_wr_enable();
    cs = 0;

    //page address starts from 000000 to 0fffff hex total 4095 pages in 1MB
    //001000 -secto1, 020000-sector 2 ---1f000 -sector 31, 02f000 sector 47

    spi.write(0x02); //page write code
    spi.write((pageaddress & 0xff00) >> 8);//(pageaddress & 0xff00) >> 8);//(pageaddress & 0xff00) >> 8); // sector address 0x7f
    spi.write((pageaddress & 0xff)); //page 16 pageaddress & 0xff
    spi.write(0x00); // start address 20 in page 16 address
    //write in page 16 start address from location 20 in that page  of sector 0
    for(int i = 0; i < 256; i++) {
        spi.write(*data++);
        printf("%c",*data);
    }
    printf("wr pageaddr %X\n",pageaddress);
    cs = 1;
    flash_wr_disable();
}
*/
/*



void test_fun(uint8_t *str)
{
    int len = strlen((char *)str);
    int8_t counter=0;
    current_bytecount_lessthan_pagesize = len % 256;
    noof_pages_towrite_frmthisblock = len / 256; //to check no.of pages
    printf("pages : %d mod : %d\n", noof_pages_towrite_frmthisblock, current_bytecount_lessthan_pagesize);
  //  flash_wr_enable();
    if(noof_pages_towrite_frmthisblock == 0) {
        flash_stringwrite(str, 0x00); //firstpage
    }
    else {
      //  flash_stringwrite(str, 0x00);
        do {
            flash_stringwrite(str+(256 * counter), 0x00 + counter);
            counter++;
          // printf("%c", *str);
        }while (counter <= 12);
    //    if(bytecount_lessthan_pagesize)
    //     flash_stringwrite(str + (256 * counter), (counter + 1));
    }
  //  flash_wr_disable();
}

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