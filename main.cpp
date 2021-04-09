/*  Modified on 8/12/2020 to work with Radio board BRD4161 */

#include "mbed.h"
#include "rtos.h"
#include "NanostackInterface.h"
#include "mbed-trace/mbed_trace.h"
#include "mesh_nvm.h"
#include <cstdint>
#include <cstring>
#include "string.h"
#include "cli_cmd.h"
#include "vmn_coap_client.h"
#include "vmn_coap_server.h"
#include "temp_humidity_sensor.h"
#include "net_thread_test.h"
#include "mx25r8035f.h"
#include "em_core.h"
#include "solenoid.h"
#include "gas_sensor.h"
#include "fan_control.h"
#include "pm_sensor_sds011.h"
//#include "thread_joiner_application.h"
//#include "thread_common.h"
/*
#define LENGTH_RAM   0x40000
#define ORIGIN_RAM   0x20000000
#define BOOT_ADDRESS    0x2003fffc
//int sign __attribute__ ((*(uint8_t *)(LENGTH(RAM) + ORIGIN(RAM) - 4)));
//int var __attribute__((at(0x40001000)));
#define SIGNITUREMSB *((volatile uint8_t *)((0x2003fffc))) // (*(uint8_t *)(LENGTH(RAM) + ORIGIN(RAM) - 4))
#define SIGNITURELSB *((volatile uint8_t *)((0x2003fffd)))
#define RESETMSB *((volatile uint8_t *)((0x2003fffe)))//*((volatile unsigned long *)((LENGTH_RAM + ORIGIN_RAM - )))
#define RESETLSB *((volatile uint8_t *)((0x2003ffff)))//*((volatile unsigned long *)((LENGTH_RAM + ORIGIN_RAM)))

#define BOOT1   *((volatile uint16_t *)((0x20000000))) // (*(uint8_t *)(LENGTH(RAM) + ORIGIN(RAM) - 4))
#define BOOT2 *((volatile uint16_t *)((0x20000002)))
#define BTL_FIRST_STAGE_BASE            *((volatile uint32_t *) 0x0FE10000UL) + 2048
#define __rom_end__              *((volatile uint32_t *)0x0FE19800UL)*/
//#define BOOT3 *((volatile uint8_t *)((0x20000002)))//*((volatile unsigned long *)((LENGTH_RAM + ORIGIN_RAM - )))
//#define BOOT4 *((volatile uint8_t *)((0x20000003)))

#if MBED_CONF_APP_ENABLE_LED_CONTROL_EXAMPLE
#include "mesh_led_control_example.h"
#endif
static Mutex SerialOutMutex;
mbed::RawSerial pc(USBTX, USBRX,115200);
mbed::RawSerial serial(USART1_TX,USART1_RX,9600);
//mbed::UARTSerial serial(USBTX, USBRX,115200);
//DigitalOut vcom_enable(PA5, 1); //added newly to enable usart
//DigitalOut led0(LED0);
//SocketAddress sockAddr;
Thread coapserver_thread;
Thread temp_hum_sensor_thread;
Thread coapclient_thread;
Thread externalflash_rdwr;
extern EventQueue coapserver_eventqueue; 
extern EventQueue sensor_eventqueue;
extern EventQueue coapclient_eventqueue;
extern EventQueue externalflash_rdwr_eventqueue;
extern volatile uint8_t flag;
extern char Rx_buff[1024 * 180];
extern char hexfile_buff[106];
extern char *coap_server_ipaddr; //check whether its needed or not
extern uint32_t Receive_buff_length;
extern uint32_t payload_length;
extern nwk_interface_id id;
extern uint8_t keeping_nw_default_details;
uint8_t parameter_test_byte = 2;
extern uint8_t flash_handler_flag;
extern uint16_t total_written_flashpages_count;
extern uint8_t flashhandle;
extern uint16_t bytecount; 

/********************************************  END of I2C Declarations   **********************/
void trace_printer(const char* str) {
    printf("%s\n", str);
}

void serial_out_mutex_wait() {
    SerialOutMutex.lock();
}

void serial_out_mutex_release() {
    SerialOutMutex.unlock();
}
extern uint16_t written_pages_count;
//const size_t __rom_end__ = 0;
///void property_fun()
//{
  /*  ApplicationData_t appdata;
    ApplicationCertificate_t *cert;
    *cert = {'\0'};
    appdata.type = APPLICATION_TYPE_BOOTLOADER;
    appdata.version = BOOTLOADER_VERSION_MAIN;
    appdata.capabilities = 0UL;
    appdata.productId[16] = {0};//{0000000000000000};
    ApplicationProperties_t properties ={APPLICATION_PROPERTIES_MAGIC,APPLICATION_PROPERTIES_VERSION,APPLICATION_SIGNATURE_NONE,\
    (((uint32_t)&__rom_end__) - BTL_MAIN_STAGE_BASE),appdata, cert};   */
 /*    ApplicationData_t appdata;
    ApplicationCertificate_t *cert;
    *cert = {'\0'};
    appdata.type = APPLICATION_TYPE_THREAD;
    appdata.version = 1;
    appdata.capabilities = 0UL;
    appdata.productId[16] = {0};//{0000000000000000};
    ApplicationProperties_t properties ={APPLICATION_PROPERTIES_MAGIC,APPLICATION_PROPERTIES_VERSION,APPLICATION_SIGNATURE_NONE,\
    (((uint32_t)&__rom_end__) - BTL_MAIN_STAGE_BASE),appdata, cert};*/
//}
/*extern uint8_t signiture[2] = {0xf0,0x0f};
extern uint8_t reset_reason[2];
signiture[0] = 0x0f;
signiture[1] = 0xf0;*/
//#define sign (*((uint8_t *) (LENGTH(RAM) + ORIGIN(RAM) - 4)))
//sign = (uint8_t )0xf0;
/*uint8_t signitureMSB __attribute__  *((uint8_t *) (LENGTH(RAM) + ORIGIN(RAM) - 4));
   uint8_t signitureLSB __attribute__ ((at((LENGTH(RAM) + ORIGIN(RAM) - 3))));
   uint8_t resetreasonMSB __attribute__ ((at((LENGTH(RAM) + ORIGIN(RAM) - 2))));
   uint8_t resetreasonLSB __attribute__ ((at((LENGTH(RAM) + ORIGIN(RAM)))));
extern uint8_t signitureMSB;
extern uint8_t signitureLSB;
extern uint8_t resetreasonMSB;
extern uint8_t resetreasonLSB;*/
uint16_t pagenumber = 0;
void test_function(char *rxdata)
{
  //  char ch = 0xeb;
 //   rxdata[1] = 'B';
 uint8_t testflag = 1;
    uint64_t len = strlen(rxdata);
    char spiData[256];
    uint16_t count=0;
 //   printf("%c %c", rxdata[0],rxdata[1]);
 //   spiData[0] = 0xeb;
  //  count = 1;
    for (uint64_t i=2 ;i < len;i++)
    {
      char temp[2] = {rxdata[i], rxdata[++i]};
      spiData[count++] = strtohex8(temp);
      if(count >= 256)
      {
          count = 0;
          mx25r8035f_pages_write((uint8_t *)spiData, 256, pagenumber++);
          if((mx25r8035f_get_status() & 0x01 ) == 0x01)
            wait_us(5000);
            testflag = 2;
      //    flashhandle = externalflash_rdwr_eventqueue.call(pages_write, (uint8_t *)spiData, 256, pagenumber++);//((uint8_t*)hexfile_buff, 256, pageaddress++);
     //     externalflash_rdwr_eventqueue.cancel(flashhandle);
      }
      if((rxdata[i] == '\0') && (count >= 1) && (testflag > 1))
      {
          printf("last");
          if((mx25r8035f_get_status() & 0x01 ) == 0x01)
            wait_us(5000);
          mx25r8035f_pages_write((uint8_t *)spiData, count, pagenumber++);
          count =0;
      }
       
     // printf("%x ",strtohex8(temp));
        
    }
}
/*
bool isInitialized1 = false;
int32_t init(void)
{
  int32_t retVal;
  bool isInitializedTemp;

  if (!bootloader_pointerValid(mainBootloaderTable)) {
    return BOOTLOADER_ERROR_INIT_TABLE;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  isInitializedTemp = isInitialized1;
  isInitialized1 = true;
  CORE_EXIT_ATOMIC();

  if (isInitializedTemp == false) {
    retVal = mainBootloaderTable->init();
  } else {
    retVal = BOOTLOADER_OK;
  }
  return retVal;
}*/

extern uint16_t pageaddress2;
void write_to_adc(void)
{
    channelwrite(MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G4);
    wait(1);
    channelwrite(MCP3423_0_ADDR, MCP3423_CH1_CONFIG_G4);
    wait(1);
    channelwrite(MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G4);
    wait(1);
    channelwrite(MCP3423_1_ADDR, MCP3423_CH1_CONFIG_G4);
}
void read_adc_x5()
{
    channelread(MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G4,sensordata_count_mcp0);
    wait(1);
    channelread(MCP3423_0_ADDR, MCP3423_CH1_CONFIG_G4,sensordata_count_mcp1);
    wait(1);
    channelread(MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G4,sensordata_count_mcp2);
    wait(1);
    channelread(MCP3423_1_ADDR, MCP3423_CH1_CONFIG_G4,sensordata_count_mcp3);
}
char payload[66];
int main() {

    DigitalOut led(LED0);
    DigitalOut red(LED1);
    DigitalIn sw0(SW0);
    printf("testing\n");
  /*  while(1)
    {
        led = !led;
        wait(10);
        red =!red;
        wait(10);

    }*/
    printf("data\n");
    i2cinit();
  /*  while(1)
    {
        if(SW0)
        {
            printf("gas data\n");
            gas_sensor();
            red =!red;
        }
    }*/
    uint8_t addr[16];
    uint8_t local_write_buffer [] ={'a'};
    int sector_Erase_increment = 0;
    int avg_o3  = 0, avg_no2 = 0;
 //   volatile unsigned int *ptr = 0x2003ffff;
   /* mbed_trace_init();
    mbed_trace_print_function_set(trace_printer);
    mbed_trace_mutex_wait_function_set(serial_out_mutex_wait);
    mbed_trace_mutex_release_function_set(serial_out_mutex_release);
    mesh = MeshInterface::get_default_instance();  //returns pointer to the mesh interface
    */
 //   thread_eui64_trace();  //This function generates the EUI64
 //   mesh_nvm_initialize();  //initializes the non-volatile memory
 //   coapserver_thread.start(callback(&coapserver_eventqueue, &EventQueue::dispatch_forever)); //coap server
 //   temp_hum_sensor_thread.start(callback(&sensor_eventqueue, &EventQueue::dispatch_forever)); //sensor data
 //   coapclient_thread.start(callback(&coapclient_eventqueue, &EventQueue::dispatch_forever)); //coapclient
  //  externalflash_rdwr.start(callback(&externalflash_rdwr_eventqueue, &EventQueue::dispatch_forever));//(hexfile_format, Rx_buff));//
 //   externalflash_rdwr.set_priority(osPriorityHigh);
    printf("Mesh changed# application\n"); //start thread mesh application red led 
 /*   SIGNITUREMSB = 0xf0;
    SIGNITURELSB = 0x0f;
    RESETMSB = 0x02;
    RESETLSB = 0x02;*/
 //   BOOT1 = 0x0202;
 //   BOOT2 = 0xf00f;
  //  BOOT3 = 0x02;
  //  BOOT4 = 0x02;

//printf("%d",(sizeof(ApplicationCertificate_t)+sizeof(ApplicationData_t)+sizeof(ApplicationProperties_t)));
 //   printf("%x %x \n",BOOT1,BOOT2);
 //   printf("0x%x 0x%x 0x%x 0x%x\n", &SIGNITUREMSB,&SIGNITURELSB,&RESETMSB,&RESETLSB);
  //  signitureMSB = 0xf0;
  /*   ptr = &SIGNITUREMSB;
    printf("%ld\n", *ptr);
    ptr = &SIGNITURELSB;
    printf("%ld \n", *ptr);
    ptr = &RESETMSB;
    printf("%ld \n", *ptr);
    ptr = &RESETLSB;
    printf("%ld \n", *ptr);*/
 //   init();
//    bootloader_setImageToBootload(0);
 // bootloader_rebootAndInstall();
    start_blinking();   //led
//    printf("test");
 //   temp_hum_sensor_read_every_5min(); //reading every 5min
    pc.attach(&isr_rx); //receive interrupt
 //   i2cinit();  //i2c frequency init
    mx25r8035f_init(); //external flash init
    total_written_flashpages_count = 0;
    int pagesize = 0;
    uint32_t pageaddress=0;
    mcp23017_config();
    fan_config();
    aqm_config();
    fan_control(fan_fulloff);
 //   property_fun();
    while (1) {
        if (flag) {
            flag = 0;
         //   printf("%s\n", Rx_buff);
        //   printf("%d\n",Receive_buff_length);
            if (Receive_buff_length > 2) {
                //This function conatains set of commands to establish mesh network connection,
                //to update parameters also to get network details.
                //Please first start with thread start command, it will establishes the network connection.
              //  cli_cmds_Handler((char *)Rx_buff);
              test_function(Rx_buff);
              /*   if(flash_handler_flag == 1)
                 {
                    flash_handler_flag = 0;
                    hexfile_format((char *)Rx_buff);
                 }*/
            }
            if(Rx_buff[0] == '4')  //assume o3
            {
           ///     fan_control(fan_speed);
           //     fan_control(fan_fullon);
               // soleniod_on();
               char buffer[10];
            //   soleniod_valve_control(solenoid_leftvalve_close);
             //  wait(1);
             soleniod_valve_control(solenoid_rightvalve_close);
             soleniod_valve_control(solenoid_leftvalve_close);
             wait(1);
               soleniod_valve_control(solenoid_rightvalve_open);
                aqm_control(aqm0_on);
                for(char i = 0; i < 5; i++) {
                   buffer[i] =  serial.getc();
                    printf(" %x ",buffer[i]); //00 aa c0 03 00 0b 00 87 c7 5c
                    //ab aa ab 00 aa c0 03 00 03 00
                }
                wait(2);
                aqm_control(aqm0_off);
                wait(5);
                soleniod_valve_control(solenoid_rightvalve_close);
                soleniod_valve_control(solenoid_leftvalve_open);
                fan_control(fan_fullon);
                wait(5); //wait for 5s
                fan_control(fan_fulloff);
                for(int i=0; i< 5; i++)
                {
                    write_to_adc();
                    wait(2);
                    read_adc_x5();
                    wait(2);
                    printf("#### \n \n \n####");
                }
                humidity_temp_read();
                soleniod_valve_control(solenoid_leftvalve_close);
                payload[0] = 4;
                payload[1] = 18;
                for(int i=0;i<5;i++)
                {
                    payload[i+2] = buffer[i];
                   
                }
                for(int i=0;i<15;i++)
                {
                    payload[i+7] = sensor_buffer_MCP0[i];
                    payload[i+7+15] = sensor_buffer_MCP1[i];
                    payload[i+7+30] = sensor_buffer_MCP2[i];
                    payload[i+7+45] = sensor_buffer_MCP3[i];
                }
                payload[61] = Temp_centigrade;
                payload[62] = RH_percentage;
             /*   wait(10);
                
                printf("after off");
                for(char i=0;i<10;i++)
                {
                    buffer[i] = pc.getc();
                    printf(" %x  ",buffer[i]);
                }
                soleniod_valve_control(solenoid_leftvalve_close);
                aqm_control(aqm0_off);*/
              //  channelwrite(MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G1);
              //  channelwrite(MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G2);
               // gas_sensor( MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G1);
           //     gas_sensor( MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G2);
           //     gas_sensor( MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G4);
           //     gas_sensor( MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G8);
             //   gas_sensor( MCP3423_0_ADDR, MCP3423_CH1_CONFIG_G1);
           //     gas_sensor( MCP3423_0_ADDR, MCP3423_CH1_CONFIG_G2);
           //     gas_sensor( MCP3423_0_ADDR, MCP3423_CH1_CONFIG_G4);
           //     gas_sensor( MCP3423_0_ADDR, MCP3423_CH1_CONFIG_G8);
                red =!red;
            }
            if(Rx_buff[0] == '5') //no2
            {
                aqm_control(aqm0_off);
              //  fan_control(fan_fulloff);
            //    soleniod_valve_control(solenoid_rightvalve_close);
               /*  channelread(MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G1,sensordata_count_mcp0);
                 sensordata_count_mcp0++;
                if(sensordata_count_mcp0 >= 5)
                 sensordata_count_mcp0 = 0;*/
           //     soleniod_off();
                
             //   gas_sensor( MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G1);
              //  gas_sensor( MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G2);
            //    gas_sensor( MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G4);
            //    gas_sensor( MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G8);
           //     gas_sensor( MCP3423_1_ADDR, MCP3423_CH1_CONFIG_G1);
              //  gas_sensor( MCP3423_1_ADDR, MCP3423_CH1_CONFIG_G2);
           //     gas_sensor( MCP3423_1_ADDR, MCP3423_CH1_CONFIG_G4);
           //     gas_sensor( MCP3423_1_ADDR, MCP3423_CH1_CONFIG_G8);
                red =!red;
            }
            if(Rx_buff[0] == '6')
            {
                soleniod_valve_control(solenoid_leftvalve_open);
              //  channelwrite(MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G1);
            }
            if(Rx_buff[0] == '7')
            {
                soleniod_valve_control(solenoid_leftvalve_close);
            //     channelread(MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G1,sensordata_count_mcp1);
                 sensordata_count_mcp1++;
                if(sensordata_count_mcp1 >= 5)
                sensordata_count_mcp1 = 0;
            }
            if(Rx_buff[0] == '8')
            {
                
                for(uint8_t i = 0;i < 10; i++)
                {
                //    printf(" %x %x ", sensor_buffer_MCP0[i], sensor_buffer_MCP1[i]);
                    avg_o3 += sensor_buffer_MCP0[i];
                    avg_no2 += sensor_buffer_MCP1[i];
                }
                printf("%d %d\n",  avg_o3,  avg_no2);
            }
            if(Rx_buff[0] == '1')
                written_pages_count = 0;
            if(Rx_buff[0] == '3') {
                uint8_t local_buff[2048];
                printf("entry: ");
             //   pagenumber = 50;
                uint8_t pagcnt = pagenumber/8;
                for(int j = 0;j<pagcnt;j++) { 
                    uint16_t cnt = mx25r8035f_page_read( local_buff, j*8 ,256);
                    printf("%d\n",cnt);
                    for(uint16_t i=0;i < cnt; i++)
                        printf("%02x ",local_buff[i]);
                }
                
            }
            if(Rx_buff[0] == '9')
            {
                humidity_temp_read();
            }
        /*    if (Rx_buff[0] == '2') {
                printf("coap server\n");
                coap_server_init(); //server init
            }
            if(Rx_buff[0] == '1') {
              parameter_test_byte = thread_management_link_configuration_delete(id); //delete the already existed parameters
              keeping_nw_default_details = 0;
            }
            if(Rx_buff[0] == '3') {
                int i = thread_management_device_type_set(id, THREAD_DEVICE_REED);
                printf("reed : %d\n", i );
            }
            if(Rx_buff[0] == '4') {
                printf("Parent Link-Local Address: ");
                thread_management_get_parent_address(id,addr);
                for (int i=0;i< 16;i++){
                    if ((i>0) && (i%2 == 0))
                        printf(":");
                printf("%02x", addr[i]);
                }
                printf("\n");
            }
            if(Rx_buff[0] == '5') {
                printf("Leader Address: ");
                thread_management_get_leader_address(id,addr);
                for (int i=0;i< 16;i++){
                    if ((i>0) && (i%2 == 0))
                        printf(":");
                printf("%02x", addr[i]);
                }
                printf("\n");
            }
            if(Rx_buff[0] == '6') {
                printf("Own Mesh-Link Address: ");
                thread_management_get_ml16_address(id,addr); //get own address
                for (int i=0;i< 16;i++){
                    if ((i>0) && (i%2 == 0))
                        printf(":");
                printf("%02x", addr[i]);
                }
                printf("\n");
            }
            if(Rx_buff[0] == '7') {
                device_configuration_s *deviceconfig;
            //    int i=thread_test_stack_cache_reset(id);
            //    int i = thread_joiner_application_link_configuration_delete(id);
           //         printf("%d\n",i);
                printf("device config \n\n");
                printf("euid64: ");
                deviceconfig = thread_management_device_configuration_get(id);
                for (int i=0;i< 8;i++){
                    printf("%x",deviceconfig->eui64[i]);
                }printf("\nmesheid:");
                for (int i=0;i< 8;i++){
                     printf("%x",deviceconfig->mesh_local_eid[i]);
                }printf("\nrandom_mac:");
                for (int i=0;i< 8;i++){
                    printf("%x",deviceconfig->extended_random_mac[i]);}printf("\n");
            }
            if(Rx_buff[0] == '8') {
                printf("child count: ");
                int i=thread_test_child_count_get(id);
                printf("%d\n",i);
            }
            if(Rx_buff[0] == '9') {
            //   int i = thread_joiner_application_nvm_link_configuration_load(id);
            //    printf("%d\n",i);
                read_device_id();
            //    read_identification();
            //    read_status_register();
            //    read_config_register();
                printf("\n\n");
            }
            if(Rx_buff[0] == 'w')  {
                //flash_pagewrite();
              //  pages_write(local_write_buffer, 4);
              //  pages_write(local_write_buffer, sizeof(local_write_buffer),4);
                coap_payload_write(8194);
            }
            if(Rx_buff[0] == 'r')  {
                //flash_read();
              //  pages_read(4,256);
              coap_payload_read(8194 , payload_length);
            }
            if(Rx_buff[0] == 'e')  {
                flash_sector_erase(sector_Erase_increment++);
              //  sector_read(sector_Erase_increment++);
            }*/

            if(Rx_buff[0] == 's')
            {
                if(pagenumber <= 0)
                    written_pages_count = 300;
                else {
                written_pages_count = pagenumber;
                }
                mx25r8035f_read();
               // written_pages_count = 0;
            }
            if(Rx_buff[0] == 'b')
            {
                mx25r8035f_block64_erase(sector_Erase_increment++);
                if(sector_Erase_increment >= 120)
                sector_Erase_increment = 0;
                pagenumber = 0;
           //     block64_read(sector_Erase_increment++);
            }
            if(Rx_buff[0] == 'c')
            {
                mx25r8035f_chip_erase();
                pageaddress2 = 0;
                pagenumber = 0;
            }
         /*  if(Rx_buff[0] == 'a')
            {
                uint8_t index;
                uint16_t *short_addr;
                bool *sleepy;
                uint8_t *mac64;
                 uint8_t *margin;
                int i=thread_test_child_info_get(id,index,short_addr,sleepy,mac64,margin);
                printf("child inforamation\n");
                printf("%d\n",i);
                printf("index:%d", index);
                while(*short_addr !='\0')
                {
                    printf("%02x", *short_addr++);
                }
                printf("\n");
                while(*mac64 !='\0')
                {
                    printf("%02x", *mac64++);
                }
                printf("\n");
                  //   addr:%02x sleepy : %d mac:%02x ")
            }*/
            

        }
    } 
}

/*

net_thread_test.h
int thread_test_remove_router_by_id(int8_t interface_id, uint8_t routerId); leader can kick out any router by ID

*/