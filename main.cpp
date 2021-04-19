/*  Modified on 8/12/2020 to work with Radio board BRD4161 */

/*
 clock frequency 40Mhz
  Time  = 1/40Mhz = 0.026us 
  1000 count = 26us
  5000 = 130us
  5000000 = 130ms
  1000000 =  0.026s = 26ms
  5000000 = 0.13s = 130ms
  38500 = 1ms
*/

#include "mbed.h"
#include "rtos.h"
#include "NanostackInterface.h"
#include "mbed-trace/mbed_trace.h"
#include "mesh_nvm.h"
#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
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
#include "realtimeclock.h"
#include "mbed_mktime.h"
#include "time.h"
#include "mesh_config.h"
//#include "thread_joiner_application.h"
//#include "thread_common.h"

#if MBED_CONF_APP_ENABLE_LED_CONTROL_EXAMPLE
#include "mesh_led_control_example.h"
#endif
static Mutex SerialOutMutex;
mbed::RawSerial pc(USBTX, USBRX,115200);
mbed::RawSerial serial(USART1_TX,USART1_RX,9600);

//DigitalOut vcom_enable(PA5, 1); //added newly to enable usart
//DigitalOut led0(LED0);
//  DigitalOut red(LED1);
//  DigitalIn sw0(SW0);
//SocketAddress sockAddr;
time_t seconds;
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
extern uint8_t flash_handler_flag;
extern char hexfile_buff[106];
extern char *coap_server_ipaddr; //check whether its needed or not
extern uint32_t Receive_buff_length;
extern uint32_t payload_length;
extern nwk_interface_id id;
extern uint8_t keeping_nw_default_details;
uint8_t parameter_test_byte = 2;
extern uint16_t total_written_flashpages_count;
extern uint8_t flashhandle;
extern uint16_t bytecount; 
extern uint8_t RH_percentage;
extern uint8_t Temp_centigrade;
extern uint16_t written_pages_count;
uint16_t pagenumber = 0;
extern uint16_t pageaddress2;
char aqmpayload[66];

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
//this function prints ipaddress on serial terminal
void print_mesh_ipaddr(uint8_t addr[16]) { 

    for (int i=0;i< 16;i++){
        if ((i>0) && (i%2 == 0))
            printf(":");
        printf("%02x", addr[i]);
    }
    printf("\n");
}

/*Calling this function writes Rx_buff data into external flash
param@ receive buffer data
This function implemented to handle the DFU firmware application.
*/
void write_recvfileinto_mx25r8035f(char *rxdata) {
    pagenumber = 1;
    char update_control_buffer[12] = {"123457890"};
    char end_of_file_control[8] = {"ota-vmn"};
    uint64_t recv_data_length = strlen(rxdata);
    uint8_t spiData[256];
    uint16_t count=0;
    mx25r8035f_singlepage_write(update_control_buffer, 9, 0);
    for (uint64_t i = 0 ;i < recv_data_length; i++)  {
        char temp[2] = {rxdata[i], rxdata[++i]};
        spiData[count++] = strtohex8(temp);
      //   printf("%x\n",spiData[count++]);
        if(count == 256) { //count 256
            count = 0;
            mx25r8035f_singlepage_write((char *)spiData, 256, pagenumber++);
            if((mx25r8035f_get_status() & 0x01 ) == 0x01)
                wait_us(5000);
              //  testflag = 2;
        } 
     }
     if( (count < 255) && (count > 0)) {
      //  else if((rxdata[i] == '\0') && (count >= 1) && (testflag > 1)) {
        printf("last");
        if((mx25r8035f_get_status() & 0x01 ) == 0x01)
            wait_us(5000);
        mx25r8035f_singlepage_write((char *)spiData, count, pagenumber++);
        count =0;
    }
    mx25r8035f_singlepage_write(end_of_file_control, 7, pagenumber);  
 //   mbed_start_application(0x00000000);
}

// delay to offer numberof delaycount
void delay_ms(uint32_t delaycount) {
    delaycount *= 38500;
    for( uint16_t i = 0; i< delaycount; i++);
}

//Here RTC is set to default system compile time
void intial_rtc_calender_set(void) {
    char timebuffer[40];
    char buffer[32];
    char *starttime = (char *)__TIME__;
    char *startdate= (char *)__DATE__;
    // printf("testing: %s **%s\n",__DATE__,__TIME__);
    strcat(timebuffer, startdate);
    strcat(timebuffer," ");
    strcat(timebuffer, starttime);
  //  printf("\n%s\n",timebuffer);
    seconds = time_in_seconds(timebuffer);
    set_time(seconds);
    seconds = time(NULL);
    struct tm timeinfo;
    _rtc_localtime(seconds, &timeinfo, RTC_FULL_LEAP_YEAR_SUPPORT);
    strftime(buffer, 32, "%I:%M %p\n", &timeinfo);
    printf("Time as a custom formatted string = %s", buffer);
}

int main() {
    uint8_t adc_write_flag = 0,adc_read_flag = 0;
    int sector_Erase_increment = 0;
    total_written_flashpages_count = 0;
    int pagesize = 0;
    uint32_t pageaddress=0;
    uint8_t addr[16];
    int8_t childcount = 0;
    intial_rtc_calender_set(); //rtc clock set to default time
    mbed_trace_init();
    mbed_trace_print_function_set(trace_printer);
    mbed_trace_mutex_wait_function_set(serial_out_mutex_wait); //to synchronize threads
    mbed_trace_mutex_release_function_set(serial_out_mutex_release); //to synchronize threads
    mesh = MeshInterface::get_default_instance();  //returns pointer to the mesh interface
    thread_eui64_trace();  //This function generates the EUI64
    mesh_nvm_initialize();  //initializes the non-volatile memory
    coapserver_thread.start(callback(&coapserver_eventqueue, &EventQueue::dispatch_forever)); //coap server
    temp_hum_sensor_thread.start(callback(&sensor_eventqueue, &EventQueue::dispatch_forever)); //sensor data
    coapclient_thread.start(callback(&coapclient_eventqueue, &EventQueue::dispatch_forever)); //coapclient
    temp_hum_sensor_thread.set_priority(osPriorityNormal);
    coapserver_thread.set_priority(osPriorityHigh);
 //   externalflash_rdwr.start(callback(&externalflash_rdwr_eventqueue, &EventQueue::dispatch_forever));//(hexfile_format, Rx_buff));//
 //   externalflash_rdwr.set_priority(osPriorityHigh);
    printf("Start of the application\n"); 
    start_blinking();   //led
    temp_hum_sensor_read_every_5min(); //reads for every 5min
    pc.attach(&isr_receive); //receive interrupt
    i2cinit();  //i2c frequency init
    mx25r8035f_init(); //external flash init
    mcp23017_config();  //I/O expander for solenoids
    fan_config();   
    pm_sensor_config();
    fan_control(fan_fulloff); 
    while (1) {
        if (flag) {
            flag = 0;
         //   printf("%s\n", Rx_buff);
        //   printf("%d\n",Receive_buff_length);
            if ((Receive_buff_length > 2) && (Receive_buff_length < 100)) {
                //This function conatains set of commands to establish mesh network connection,
                //to update parameters also to get network details.
                //Please first start with thread start command, it will establishes the network connection.
                cli_cmds_Handler((char *)Rx_buff);
              
              /*   if(flash_handler_flag == 1)
                 {
                    flash_handler_flag = 0;
                    hexfile_format((char *)Rx_buff);
                 }*/
            } else if (Receive_buff_length > 100) { //just for test to test external flash read and write
                write_recvfileinto_mx25r8035f(Rx_buff);
            }
            else {
                switch(Rx_buff[0]) {
                    case '0':  //prints ipaddresses
                        printf("Parent Link-Local Address: ");
                        thread_management_get_parent_address(id,addr);
                        print_mesh_ipaddr(addr);
                        printf("Leader Address: ");
                        thread_management_get_leader_address(id,addr);
                        print_mesh_ipaddr(addr);
                        printf("Own Mesh-Link Address: ");
                        thread_management_get_ml16_address(id,addr); //get own address
                        print_mesh_ipaddr(addr);
                    break;
                    case '1': //deletes current network config details
                        parameter_test_byte = thread_management_link_configuration_delete(id); //delete the already existed parameters
                        keeping_nw_default_details = 0;
                    break;
                    case '2': 
                        printf("Device type : %d\n", thread_management_device_type_set(id, THREAD_DEVICE_REED));
                    break;
                    case '3': //coapserver init
                        printf("coap server\n");
                        coap_server_init(); 
                    break;
                    case '4': //child count
                        printf("child count :");
                        childcount = thread_test_child_count_get(id);
                        printf("%d\n",childcount);
                    break;
                    case '5': //pm sensor data capture, fan on/off, solenoid open/close
                        char pmsensor_output[10];
                    //    fan_control(fan_speed);
                    //     fan_control(fan_fullon);
                    //    soleniod_on();
                    //   soleniod_valve_control(solenoid_leftvalve_close);
                    //  wait(1);
                        soleniod_valve_control(solenoid_rightvalve_close);
                        soleniod_valve_control(solenoid_leftvalve_close);
                        delay_ms(3000);    //allow some time before reopen the valve
                        soleniod_valve_control(solenoid_rightvalve_open);
                        pm_sensor_control(pm_sensor_on);
                        for(char i = 0; i < 5; i++) {
                        pmsensor_output[i] =  serial.getc();
                            printf(" %x ",pmsensor_output[i]); //00 aa c0 03 00 0b 00 87 c7 5c
                            //ab aa ab 00 aa c0 03 00 03 00
                        }
                        delay_ms(2000);
                        pm_sensor_control(pm_sensor_off);
                        delay_ms(5000);
                        soleniod_valve_control(solenoid_rightvalve_close);
                        aqmpayload[0] = 4;      //gain
                        aqmpayload[1] = 18;     //sample rate
                        for(int8_t i = 0; i < 5; i++) {
                            aqmpayload[i+2] = pmsensor_output[i];
                        }
                        soleniod_valve_control(solenoid_leftvalve_open);
                        fan_control(fan_fullon);
                        delay_ms(5000); //wait for 5s
                        fan_control(fan_fulloff);
                    break;
                    case '6': //ADC channel write
                        adc_write_flag++;
                        switch(adc_write_flag) {
                            case 1: case 5:case 9: case 13: case 17:
                            adcchannelwrite(MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G8);
                            break;
                            case 2: case 6:case 10: case 14: case 18:
                            adcchannelwrite(MCP3423_0_ADDR, MCP3423_CH1_CONFIG_G8);
                            break;
                            case 3: case 7:case 11: case 15: case 19:
                            adcchannelwrite(MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G8);
                            break;
                            case 4: case 8:case 12: case 16: case 20:
                            adcchannelwrite(MCP3423_1_ADDR, MCP3423_CH1_CONFIG_G8);
                            break;
                            default: adc_write_flag=0;break;
                        }
                        if(adc_write_flag == 20)
                            adc_write_flag = 0;
                    break;
                    case '7': //ADC channel read
                        adc_read_flag++;
                        switch(adc_read_flag) {
                            case 1: case 5:case 9: case 13: case 17:
                            adcchannelread(MCP3423_0_ADDR, MCP3423_CH0_CONFIG_G8,sensordata_count_mcp0);
                            break;
                            case 2: case 6:case 10: case 14: case 18:
                            adcchannelread(MCP3423_0_ADDR, MCP3423_CH1_CONFIG_G8,sensordata_count_mcp1);
                            break;
                            case 3: case 7:case 11: case 15: case 19:
                            adcchannelread(MCP3423_1_ADDR, MCP3423_CH0_CONFIG_G8,sensordata_count_mcp2);
                            break;
                            case 4: case 8:case 12: case 16: case 20:
                            adcchannelread(MCP3423_1_ADDR, MCP3423_CH1_CONFIG_G8,sensordata_count_mcp3);
                            break;
                            default: adc_read_flag=0;break;
                        }
                        if(adc_read_flag == 20)
                            adc_read_flag = 0;
                    break;
                    case '8': //This case prepares AQM payload
                        soleniod_valve_control(solenoid_leftvalve_close);
                        humidity_temp_read();
                        for( int i = 0; i < 15; i++ )  {
                            aqmpayload[i+7] = sensor_buffer_MCP0[i];
                            aqmpayload[i+7+15] = sensor_buffer_MCP1[i];
                            aqmpayload[i+7+30] = sensor_buffer_MCP2[i];
                            aqmpayload[i+7+45] = sensor_buffer_MCP3[i];
                        }
                        aqmpayload[61] = Temp_centigrade;
                        aqmpayload[62] = RH_percentage;
                        
                    break;
                    case '9': //RTC read
                        char buffer[32];
                        struct tm time_info;
                        seconds = time(NULL);
                    //  printf("testing: %s **%s\n",__DATE__,__TIME__);
                        _rtc_localtime(seconds, &time_info, RTC_FULL_LEAP_YEAR_SUPPORT);
                    //    printf("Time as seconds %d\n", seconds);
                        printf("Time as a basic string = %s", ctime(&seconds));
                        strftime(buffer, 32, "%I:%M %p\n", &time_info);
                        printf("Time as a custom formatted string = %s", buffer);
                    break;
                    
                    case 'a':
                        written_pages_count = 0; //this is pagecount for external flash added for DFU
                    break;
                    case 'b': //block erase
                        mx25r8035f_block64_erase(sector_Erase_increment++);
                        if(sector_Erase_increment >= 120)
                        sector_Erase_increment = 0;
                        pagenumber = 0;
                    //  block64_read(sector_Erase_increment++);
                    break;
                    case 'c': //chip erase
                        mx25r8035f_chip_erase();
                        pageaddress2 = 0;
                        pagenumber = 0;
                    break;
                    case 'r': //block erase
                        if(pagenumber <= 0)
                            written_pages_count = 300; //minimum page read just using for test purpose
                        else {
                        written_pages_count = pagenumber; //actual number of written pages
                        }
                        mx25r8035f_read();
                    // written_pages_count = 0;
                    break;
                }

            }
        }
    } 
}

       /*       if(Rx_buff[0] == '7') {
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
                //   int i = thread_joiner_application_nvm_link_configuration_load(id);
                //    printf("%d\n",i);
                    read_device_id();
                //    read_identification();
                //    read_status_register();
                //    read_config_register();
                    printf("\n\n");
                }

            if(Rx_buff[0] == 'a')
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
/*

net_thread_test.h
int thread_test_remove_router_by_id(int8_t interface_id, uint8_t routerId); leader can kick out any router by ID

*/