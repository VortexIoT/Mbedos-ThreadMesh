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

#include <errno.h>
#include <functional>

#include "BlockDevice.h"

// Maximum number of elements in buffer
#define BUFFER_MAX_LEN 10
#define FORCE_REFORMAT true
// This will take the system's default block device
BlockDevice *bd = new HeapBlockDevice(2048, 1, 1, 512);

// Instead of the default block device, you can define your own block device.
// For example: HeapBlockDevice with size of 2048 bytes, read size 1, write size 1 and erase size 512.
// #include "HeapBlockDevice.h"
// BlockDevice *bd = new HeapBlockDevice(2048, 1, 1, 512);


// This example uses LittleFileSystem as the default file system
#include "LittleFileSystem.h"
LittleFileSystem fs("fs",bd,64,64,512,512);


#if MBED_CONF_APP_ENABLE_LED_CONTROL_EXAMPLE
#include "mesh_led_control_example.h"
#endif

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
void serial_out_mutex_wait() {
    SerialOutMutex.lock();
}
void serial_out_mutex_release() {
    SerialOutMutex.unlock();
}
uint8_t block[512] = "Hello World!\n";
int main() {
    uint8_t buff[]  = "time_to_test";
    uint8_t delete_buf[] = "abcd";
    SocketAddress addr;
    mbed_trace_init();
    mbed_trace_print_function_set(trace_printer);
    mbed_trace_mutex_wait_function_set(serial_out_mutex_wait);
    mbed_trace_mutex_release_function_set(serial_out_mutex_release);
    printf("Start Thread - Mesh application\n");
    start_blinking();

    printf("Initializing the block device... ");
    fflush(stdout);
    int err = bd->init();
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }
    bd->erase(0, bd->size());
   // bd->program(block, 0, 512);
  //  bd->read(block, 0, 512);
    
    bd->deinit();

    printf("Mounting the filesystem... ");
    fflush(stdout);
    int err1 = fs.mount(bd);
    printf("%s\n", (err1 ? "Fail :(" : "OK"));
    if (err1 || FORCE_REFORMAT) {
        // Reformat if we can't mount the filesystem
        printf("formatting... ");
        fflush(stdout);
        err1 = fs.reformat(bd);
        printf("%s\n", (err1 ? "Fail :(" : "OK"));
        if (err1) {
            error("error: %s (%d)\n", strerror(-err1), err1);
        }
        int err1 = fs.mount(bd);
    printf("%s\n", (err1 ? "Fail :(" : "OK"));
    }

      // Open the numbers file
    printf("Opening \"/fs/numbers.txt\"... ");
    fflush(stdout);
    FILE *f = fopen("/fs/numbers.txt", "r+");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) {
        // Create the numbers file if it doesn't exist
        printf("No file found, creating a new file... ");
        fflush(stdout);
        f = fopen("/fs/numbers.txt", "w+");
        printf("%s\n", (!f ? "Fail :(" : "OK"));
     //   if (!f) {
       //     error("error: %s (%d)\n", strerror(errno), -errno);
      //  }

        for (int i = 0; i < 10; i++) {
            printf("\rWriting numbers (%d/%d)... ", i, 10);
            fflush(stdout);
            err = fprintf(f, "    %d\n", i);
            if (err < 0) {
                printf("Fail :(\n");
            //    error("error: %s (%d)\n", strerror(errno), -errno);
            }
            printf("i count\n");
        }
        printf("\rWriting numbers (%d/%d)... OK\n", 10, 10);

    //    printf("Seeking file... ");
     //   fflush(stdout);
     //   err = fseek(f, 0, SEEK_SET);
    //    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
     //   if (err < 0) {
      //      error("error: %s (%d)\n", strerror(errno), -errno);
      //  }
    }

    // Go through and increment the numbers
  /*  for (int i = 0; i < 10; i++) {
        printf("\rIncrementing numbers (%d/%d)... ", i, 10);
        fflush(stdout);

        // Get current stream position
        long pos = ftell(f);

        // Parse out the number and increment
        char buf[BUFFER_MAX_LEN];
        if (!fgets(buf, BUFFER_MAX_LEN, f)) {
            error("error: %s (%d)\n", strerror(errno), -errno);
        }
        char *endptr;
        int32_t number = strtol(buf, &endptr, 10);
        if (
            (errno == ERANGE) || // The number is too small/large
            (endptr == buf) ||   // No character was read
            (*endptr && *endptr != '\n') // The whole input was not converted
        ) {
            continue;
        }
        number += 1;

        // Seek to beginning of number
        fseek(f, pos, SEEK_SET);
    
        // Store number
        fprintf(f, "    %d\n", number);

        // Flush between write and read on same file
        fflush(f);
    }*/
    printf("\rIncrementing numbers (%d/%d)... OK\n", 10, 10);

    // Close the file which also flushes any cached writes
    printf("Closing \"/fs/numbers.txt\"... ");
    fflush(stdout);
    err = fclose(f);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
    //    error("error: %s (%d)\n", strerror(errno), -errno);
    }
    
    // Display the root directory
    printf("Opening the root directory... ");
    fflush(stdout);
    DIR *d = opendir("/fs/");
    printf("%s\n", (!d ? "Fail :(" : "OK"));
    if (!d) {
      //  error("error: %s (%d)\n", strerror(errno), -errno);
    }

    printf("root directory:\n");
    while (true) {
        struct dirent *e = readdir(d);
        if (!e) {
            break;
        }

        printf("    %s\n", e->d_name);
    }

    printf("Closing the root directory... ");
    fflush(stdout);
    err = closedir(d);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
     //   error("error: %s (%d)\n", strerror(errno), -errno);
    }

    // Display the numbers file
    printf("Opening \"/fs/numbers.txt\"... ");
    fflush(stdout);
    f = fopen("/fs/numbers.txt", "r");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) {
     //   error("error: %s (%d)\n", strerror(errno), -errno);
    }

    printf("numbers:\n");
    while (!feof(f)) {
        int c = fgetc(f);
        printf("%c", c);
    }

    printf("\rClosing \"/fs/numbers.txt\"... ");
    fflush(stdout);
    err = fclose(f);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
   //     error("error: %s (%d)\n", strerror(errno), -errno);
    }

    // Tidy up
    printf("Unmounting... ");
    fflush(stdout);
    err = fs.unmount();
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(-err), err);
    }
        
    printf("Mbed OS filesystem example done!\n");
    printf("Deinitializing the block device... ");
    fflush(stdout);
    bd->deinit();
   // pc.attach(&isr_rx);
 //   mesh_nvm_initialize();
    while(1);
    while (1) {
        if (flag) {
            flag = 0;
            printf("> %s\n", Rx_buff);
            if(Receive_buff_length > 1)
            {
                
                //This function conatains set of commands to establish mesh network connection,
                //to update parameters also to get network details.
                //Please first start with thread start command, it will establishes the network connection.
                cli_cmds_Handler((char *)Rx_buff);
            }
          //  socket->sigio(mbed::callback(nsapi_dns_query_async_socket_callback, query->socket_cb_data));
        /*    else
            {
                switch (Rx_buff[0]) {//This case is for test purpose
                    case '1': humidity_temp_read();
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
                    default: //cli_cmds_Handler((char *)Rx_buff);
                    break;
                }
            }*/
        }
    } 
}
