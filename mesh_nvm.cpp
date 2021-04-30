/*
 * Copyright (c) 2018 ARM Limited. All rights reserved.
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
#include "Nanostack.h"
#include <stdio.h>

/* Application configuration values from json */
#define MESH_NVM_HEAP       1
#define MESH_NVM_SD_CARD    2
#define MESH_NVM_NONE       3

/* At the moment, Thread builds using K64F support NVM */
//#if defined MBED_CONF_APP_STORAGE_DEVICE && MBED_CONF_APP_STORAGE_DEVICE != MESH_NVM_NONE && defined(TARGET_K64F)

#include "LittleFileSystem.h"
//#include "SDBlockDevice.h"
#include "HeapBlockDevice.h"
#include "ns_file_system.h"
#include "mbed_trace.h"
/*
#define TRACE_GROUP "mnvm"
#define BLOCK_SIZE 512
//LittleFileSystem *fs;
//BlockDevice *bd;
// Maximum number of elements in buffer
#define BUFFER_MAX_LEN 10
#define FORCE_REFORMAT true
// This will take the system's default block device
BlockDevice *bd = new HeapBlockDevice(2048, 1, 1, 512);

#include "LittleFileSystem.h"
LittleFileSystem fs("fs");


void mesh_nvm_initialize(){
    // Try to mount the filesystem
    printf("Mounting the filesystem... ");
    fflush(stdout);
    int err = fs.mount(bd);
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err || FORCE_REFORMAT) {
        // Reformat if we can't mount the filesystem
        printf("formatting... ");
        fflush(stdout);
        err = fs.reformat(bd);
        printf("%s\n", (err ? "Fail :(" : "OK"));
        if (err) {
            error("error: %s (%d)\n", strerror(-err), err);
        }
    }
}*/
/*
//HeapBlockDevice bd(2048, BLOCK_SIZE);; //= BlockDevice::get_default_instance();
uint8_t block[BLOCK_SIZE] = "Hello World!\n";
void mesh_nvm_initialize()
{
    fs = new LittleFileSystem("fs");
//#if MBED_CONF_APP_STORAGE_DEVICE == MESH_NVM_HEAP
 
 //  HeapBlockDevice bd(2048, BLOCK_SIZE);
  //   bd.erase(0, BLOCK_SIZE);
 //     bd.program(block, 0, BLOCK_SIZE);
  //    bd.read(block, 0, BLOCK_SIZE);
  //    printf("%s", block);
  //    bd.deinit();
printf("nvm heap\n");
    const char *bd_info = "NVM: Heap";
    bd = new HeapBlockDevice(16 * 512, 512);
  //  bd->init();
 //    bd->erase(0, BLOCK_SIZE);
 //     bd->program(block, 0, BLOCK_SIZE);
  //    bd->read(block, 0, BLOCK_SIZE);
  //    printf("%s", block);
     
//#else
 //   const char *bd_info = "NVM: SD";
 //   bd = new SDBlockDevice(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO, MBED_CONF_SD_SPI_CLK, MBED_CONF_SD_SPI_CS);
//#endif

    printf("%s", bd_info);    //tr_debug("%s", bd_info);
    int mount_status = fs->mount(bd);
    if (mount_status) {
        printf("mount error: %d, trying format...", mount_status); //tr_warning
        mount_status = fs->reformat(bd);
        printf("reformat %s (%d)", mount_status ? "failed" : "OK", mount_status); //tr_info
    }

    if (!mount_status) {
        Nanostack::get_instance(); // ensure Nanostack is initialised
        ns_file_system_set_root_path("/fs/");
        // Should be like: Nanostack::get_instance().file_system_set_root_path("/fs/");
    }
}*/

//#else /* #if defined MBED_CONF_APP_STORAGE_DEVICE && MBED_CONF_APP_STORAGE_DEVICE != MESH_NVM_NONE && defined(TARGET_K64F) */



//#endif  /* #if defined MBED_CONF_APP_STORAGE_DEVICE && MBED_CONF_APP_STORAGE_DEVICE != MESH_NVM_NONE && defined(TARGET_K64F) */
/*
#include "lfs.h"

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read  = user_provided_block_device_read,
    .prog  = user_provided_block_device_prog,
    .erase = user_provided_block_device_erase,
    .sync  = user_provided_block_device_sync,

    // block device configuration
    .read_size = 16,
    .prog_size = 16,
    .block_size = 4096,
    .block_count = 128,
    .lookahead = 128,
};

// entry point
int main(void) {
    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // release any resources we were using
    lfs_unmount(&lfs);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
}*/

void mesh_nvm_initialize()
{
    /* NVM not supported */
}