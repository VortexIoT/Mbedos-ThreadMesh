/***************************************************************************//**
 * @file PinNames.h
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/
#ifndef MBED_PINNAMES_H
#define MBED_PINNAMES_H

#include "CommonPinNames.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EFM32_STANDARD_PIN_DEFINITIONS,

    /* MGM12P pins */
    LED0 = PC8,
    LED1 = PB13,
    LED2 = LED0,
    LED3 = LED0,
    LED4 = LED1,

    /* Push Buttons */
    SW0 = PB11,
    SW1 = PC9,
    BTN0 = SW0,
    BTN1 = SW1,
    // Standardized button names
    BUTTON1 = BTN0,
    BUTTON2 = BTN1,

    /* Serial (just some usable pins) */
  //  SERIAL_TX   = PD10,
 //   SERIAL_RX   = PD11,

    /* Board Controller UART (USB)*/
    USBTX       = PA1,
    USBRX       = PA0,
    

    /* Board Controller */
    STDIO_UART_TX = USBTX,
    STDIO_UART_RX = USBRX,

  //  I2C0_SDA = PC10,
  //  I2C0_SCL = PC11,
    USART1_TX = PC6,
    USART1_RX = PC7,
    
    I2C0_SDA = PA4,
    I2C0_SCL = PA5,

    FLASH_SCLK = PF5,
    FLASH_MOSI = PF7,
    FLASH_CS = PF4,
    FLASH_MISO = PF6  
} PinName;

#ifdef __cplusplus
}
#endif

#endif
