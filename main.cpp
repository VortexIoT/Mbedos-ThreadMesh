/*
 * Copyright (c) 2016 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mbed.h"
#include "rtos.h"
#include "NanostackInterface.h"
#include "mbed-trace/mbed_trace.h"
#include "mesh_nvm.h"
#include <cstring>
#include "string.h"
#include "cli_cmd.h"
#include "temp_humidity_sensor.h"
#include "vmn_coap_server.h"
#include "vmn_coap_client.h"

#if MBED_CONF_APP_ENABLE_LED_CONTROL_EXAMPLE
#include "mesh_led_control_example.h"
#endif

static Mutex SerialOutMutex;
extern mbed::RawSerial pc;
Thread coapserver_thread;
Thread temp_hum_sensor_thread;
Thread coapclient_thread;
extern EventQueue coapserver_eventqueue; 
extern EventQueue sensor_eventqueue;
extern EventQueue coapclient_eventqueue;

extern volatile uint8_t flag;
extern volatile char Rx_buff[256];
extern char *coap_server_ipaddr; //check whether its needed or not
//SocketAddress sockAddr;
extern uint8_t Receive_buff_length;


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
  //  SocketAddress addr;
   // mbed_trace_init();
 //   mbed_trace_print_function_set(trace_printer);
 //   mbed_trace_mutex_wait_function_set(serial_out_mutex_wait);
  //  mbed_trace_mutex_release_function_set(serial_out_mutex_release);
    printf("Start Thread - Mesh application\n");
    while(1);
 /*   coapserver_thread.start(callback(&coapserver_eventqueue, &EventQueue::dispatch_forever)); //coap server
    temp_hum_sensor_thread.start(callback(&sensor_eventqueue, &EventQueue::dispatch_forever)); //sensor data
    coapclient_thread.start(callback(&coapclient_eventqueue, &EventQueue::dispatch_forever)); //coapclient*/
    start_blinking();   //led
    temp_hum_sensor_read_every_5min(); //reading every 5min
    pc.attach(&isr_rx); //receive interrupt
    i2cinit();  //i2c frequency init
 /*   while (1) {
        if (flag) {
            flag = 0;
            printf("> %s\n", Rx_buff);
            if (Receive_buff_length > 1) {
                //This function conatains set of commands to establish mesh network connection,
                //to update parameters also to get network details.
                //Please first start with thread start command, it will establishes the network connection.
                cli_cmds_Handler((char *)Rx_buff);
            }
            if (Rx_buff[0] == '2') {
                coap_server_init(); //server init
            }
        }
    } */
}
