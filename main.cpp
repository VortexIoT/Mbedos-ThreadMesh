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
#include "Clicmd.h"
#include "RH_TEMP.h"
#include "CoAP.h"


#if MBED_CONF_APP_ENABLE_LED_CONTROL_EXAMPLE
#include "mesh_led_control_example.h"
#endif

//MeshInterface *mesh;
static Mutex SerialOutMutex;
extern mbed::RawSerial pc;
extern volatile uint8_t flag;
extern volatile char Rx_buff[256];
//SocketAddress sockAddr;
extern uint8_t UDP_WR_RD(char *hostdomain, uint16_t port,uint8_t *msg, uint16_t msglen);
extern uint8_t Receive_buff_length;
/********************************************  END of I2C Declarations   **********************/
void trace_printer(const char* str) {
    printf("%s\n", str);
}
void serial_out_mutex_wait()
{
    SerialOutMutex.lock();
}

void serial_out_mutex_release()
{
    SerialOutMutex.unlock();
}
int main()
{
    int i=1;
     uint8_t buff[]  = "time_to_test";
    mbed_trace_init();
    mbed_trace_print_function_set(trace_printer);
    mbed_trace_mutex_wait_function_set( serial_out_mutex_wait );
    mbed_trace_mutex_release_function_set( serial_out_mutex_release );

    printf("Start Thread - Mesh application\n");

    start_blinking();
    pc.attach(&ISR_Rx);
    while(1)
    {
        if(flag)
        {
            flag = 0;
            printf("> %s\n", Rx_buff);
            if(Receive_buff_length > 1)
            {
                
                //This function conatains set of commands to establish mesh network connection,
                //to update parameters also to get network details.
                //Please first start with thread start command, it will establishes the network connection.
                Clicmd_format_making((char *)Rx_buff);
            }

            if(Rx_buff[0] == '1')  //to read Temperature and Humidity
            {
                Humidity_Temp_Read();
            }
            if(Rx_buff[0] == '2') //to send and receive buff data on UDP
                UDP_WR_RD("test", 1234 ,buff, sizeof(buff));         

        }
    } 
}
