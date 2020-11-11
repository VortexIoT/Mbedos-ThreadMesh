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
#include "cli_cmd.h"
#include "temp_humidity_sensor.h"
#include "coap_protocol.h"
#include <cstring>
#include "string.h"


#if MBED_CONF_APP_ENABLE_LED_CONTROL_EXAMPLE
#include "mesh_led_control_example.h"
#endif

//MeshInterface *mesh;
static Mutex SerialOutMutex;
extern mbed::RawSerial pc;
extern volatile uint8_t flag;
extern volatile char Rx_buff[256];
//SocketAddress sockAddr;

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
 
    uint8_t buff[]  = "time_to_test";
    uint8_t delete_buf[] = "abcd";
     SocketAddress addr;
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
         //   if(Receive_buff_length > 1)
            {
                
                //This function conatains set of commands to establish mesh network connection,
                //to update parameters also to get network details.
                //Please first start with thread start command, it will establishes the network connection.
            //    cli_cmds_Handler((char *)Rx_buff);
            }
           // else
            {
                switch(Rx_buff[0]) //This case is for test purpose
                {
                    case '1': Humidity_Temp_Read();
                    break;
                    case '2': //UDP_rd_wr(5683, buff, sizeof(buff));      
                    break;
                    case '3':   getfunction();
                    break;
                    case '4': //get
                        coap_init(COAP_MSG_CODE_REQUEST_GET, COAP_MSG_TYPE_CONFIRMABLE,NULL, 0);
                    break;
                    case '5'://put
                        coap_init(COAP_MSG_CODE_REQUEST_PUT, COAP_MSG_TYPE_CONFIRMABLE,buff, sizeof(buff));
                    break;
                    case '6'://post4
                        coap_init(COAP_MSG_CODE_REQUEST_POST, COAP_MSG_TYPE_CONFIRMABLE,buff, sizeof(buff));
                    break;//delete
                    case '7':
                        coap_init(COAP_MSG_CODE_REQUEST_DELETE, COAP_MSG_TYPE_CONFIRMABLE, delete_buf, sizeof(delete_buf));
                    break;
                    case '8':
                        coap_init(COAP_MSG_CODE_REQUEST_DELETE, COAP_MSG_TYPE_CONFIRMABLE, NULL, 0);
                    break;
                    default: cli_cmds_Handler((char *)Rx_buff);
                    break;
                }
            }
        }
    } 
}
