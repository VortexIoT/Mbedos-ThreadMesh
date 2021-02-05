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
//#include "thread_joiner_application.h"
//#include "thread_common.h"

#if MBED_CONF_APP_ENABLE_LED_CONTROL_EXAMPLE
#include "mesh_led_control_example.h"
#endif

static Mutex SerialOutMutex;
mbed::RawSerial pc(USBTX, USBRX,115200);
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
extern char Rx_buff[1024];
extern char hexfile_buff[1024];
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

int main() {
    uint8_t addr[16];
    uint8_t local_write_buffer [] ={'a'};
    int sector_Erase_increment = 0;
    mbed_trace_init();
    mbed_trace_print_function_set(trace_printer);
    mbed_trace_mutex_wait_function_set(serial_out_mutex_wait);
    mbed_trace_mutex_release_function_set(serial_out_mutex_release);
    mesh = MeshInterface::get_default_instance();  //returns pointer to the mesh interface
    thread_eui64_trace();  //This function generates the EUI64
    mesh_nvm_initialize();  //initializes the non-volatile memory
    coapserver_thread.start(callback(&coapserver_eventqueue, &EventQueue::dispatch_forever)); //coap server
    temp_hum_sensor_thread.start(callback(&sensor_eventqueue, &EventQueue::dispatch_forever)); //sensor data
    coapclient_thread.start(callback(&coapclient_eventqueue, &EventQueue::dispatch_forever)); //coapclient
    externalflash_rdwr.start(callback(&externalflash_rdwr_eventqueue, &EventQueue::dispatch_forever));//(hexfile_format, Rx_buff));//
    printf("Start Thread - Mesh application\n");
    start_blinking();   //led
    temp_hum_sensor_read_every_5min(); //reading every 5min
    pc.attach(&isr_rx); //receive interrupt
    i2cinit();  //i2c frequency init
    mx25r8035f_init(); //external flash init
    total_written_flashpages_count = 0;
    int pagesize = 0;
    uint32_t pageaddress=0;
    while (1) {
        if (flag) {
            flag = 0;
           // printf("> %s\n", Rx_buff);
        //   printf("%d\n",Receive_buff_length);
            if (Receive_buff_length > 2) {
                //This function conatains set of commands to establish mesh network connection,
                //to update parameters also to get network details.
                //Please first start with thread start command, it will establishes the network connection.
                cli_cmds_Handler((char *)Rx_buff);
                 if(flash_handler_flag == 1)
                 {
                    flash_handler_flag = 0;
                    hexfile_format((char *)Rx_buff);
                 }
            }
            if (Rx_buff[0] == '2') {
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
            }

            if(Rx_buff[0] == 's')
            {
                mx25r8035f_read();
            }
            if(Rx_buff[0] == 'b')
            {
                block64_erase(sector_Erase_increment++);
                if(sector_Erase_increment >= 120)
                sector_Erase_increment = 0;
           //     block64_read(sector_Erase_increment++);
            }
            if(Rx_buff[0] == 'c')
                chip_erase();
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