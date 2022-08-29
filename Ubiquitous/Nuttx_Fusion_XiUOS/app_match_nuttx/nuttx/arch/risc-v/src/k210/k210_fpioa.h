/****************************************************************************
 * arch/risc-v/src/k210/k210_fpioa.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/**
* @file k210_fpioa.h
* @brief nuttx source code
*                https://github.com/apache/incubator-nuttx.git
* @version 10.3.0 
* @author AIIT XUOS Lab
* @date 2022-03-23
*/

#ifndef __ARCH_RISCV_SRC_K210_K210_FPIOA_H
#define __ARCH_RISCV_SRC_K210_K210_FPIOA_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define K210_IO_NUMBER 48
#define K210_GPIOHS_MAX_PINNO  32
#define K210_GPIO_MAX_PINNO    8

#define K210_IO_FUNC_UARTHS_RX 18  /* UART High speed Receiver */
#define K210_IO_FUNC_UARTHS_TX 19  /* UART High speed Transmitter */
#define K210_IO_FUNC_GPIOHS0   24  /* GPIO High speed 0 */
#define K210_IO_FUNC_GPIOHS1   25  /* GPIO High speed 1 */
#define K210_IO_FUNC_GPIOHS2   26  /* GPIO High speed 2 */
#define K210_IO_FUNC_GPIOHS3   27  /* GPIO High speed 3 */
#define K210_IO_FUNC_GPIOHS4   28  /* GPIO High speed 4 */
#define K210_IO_FUNC_GPIOHS5   29  /* GPIO High speed 5 */
#define K210_IO_FUNC_GPIOHS6   30  /* GPIO High speed 6 */
#define K210_IO_FUNC_GPIOHS7   31  /* GPIO High speed 7 */
#define K210_IO_FUNC_GPIOHS8   32  /* GPIO High speed 8 */
#define K210_IO_FUNC_GPIOHS9   33  /* GPIO High speed 9 */
#define K210_IO_FUNC_GPIOHS10  34  /* GPIO High speed 10 */
#define K210_IO_FUNC_GPIOHS11  35  /* GPIO High speed 11 */
#define K210_IO_FUNC_GPIOHS12  36  /* GPIO High speed 12 */
#define K210_IO_FUNC_GPIOHS13  37  /* GPIO High speed 13 */
#define K210_IO_FUNC_GPIOHS14  38  /* GPIO High speed 14 */
#define K210_IO_FUNC_GPIOHS15  39  /* GPIO High speed 15 */
#define K210_IO_FUNC_GPIOHS16  40  /* GPIO High speed 16 */
#define K210_IO_FUNC_GPIOHS17  41  /* GPIO High speed 17 */
#define K210_IO_FUNC_GPIOHS18  42  /* GPIO High speed 18 */
#define K210_IO_FUNC_GPIOHS19  43  /* GPIO High speed 19 */
#define K210_IO_FUNC_GPIOHS20  44  /* GPIO High speed 20 */
#define K210_IO_FUNC_GPIOHS21  45  /* GPIO High speed 21 */
#define K210_IO_FUNC_GPIOHS22  46  /* GPIO High speed 22 */
#define K210_IO_FUNC_GPIOHS23  47  /* GPIO High speed 23 */
#define K210_IO_FUNC_GPIOHS24  48  /* GPIO High speed 24 */
#define K210_IO_FUNC_GPIOHS25  49  /* GPIO High speed 25 */
#define K210_IO_FUNC_GPIOHS26  50  /* GPIO High speed 26 */
#define K210_IO_FUNC_GPIOHS27  51  /* GPIO High speed 27 */
#define K210_IO_FUNC_GPIOHS28  52  /* GPIO High speed 28 */
#define K210_IO_FUNC_GPIOHS29  53  /* GPIO High speed 29 */
#define K210_IO_FUNC_GPIOHS30  54  /* GPIO High speed 30 */
#define K210_IO_FUNC_GPIOHS31  55  /* GPIO High speed 31 */
#define K210_IO_FUNC_GPIO0     56  /* GPIO pin 0 */
#define K210_IO_FUNC_GPIO1     57  /* GPIO pin 1 */
#define K210_IO_FUNC_GPIO2     58  /* GPIO pin 2 */
#define K210_IO_FUNC_GPIO3     59  /* GPIO pin 3 */
#define K210_IO_FUNC_GPIO4     60  /* GPIO pin 4 */
#define K210_IO_FUNC_GPIO5     61  /* GPIO pin 5 */
#define K210_IO_FUNC_GPIO6     62  /* GPIO pin 6 */
#define K210_IO_FUNC_GPIO7     63  /* GPIO pin 7 */

#define K210_IO_DS(x)  (x << 8) /* Driving Selector */

#define K210_IO_OUTPUT_ENABLE  (1 << 12)
#define K210_IO_OUTPUT_INVERT  (1 << 13)
#define K210_IO_INPUT_ENABLE   (1 << 20)
#define K210_IO_INPUT_INVERT   (1 << 21)
#define K210_IO_PULL_DOWN      (1 << 16)
#define K210_IO_PULL_UP        (3 << 16)
#define K210_IO_PULL_UP_STRONG (7 << 16)
#define K210_IO_SL             (1 << 19)
#define K210_IO_ST             (1 << 23)

#define K210_IOFLAG_GPIOHS (K210_IO_DS(0xf) | K210_IO_OUTPUT_ENABLE | \
                            K210_IO_INPUT_ENABLE | K210_IO_ST)

typedef struct _fpioa_io_config
{
    uint32_t ch_sel : 8;  /* Channel select from 256 input. */
    uint32_t ds : 4;      /* Driving selector. */
    uint32_t oe_en : 1;   /* Static output enable, will AND with OE_INV. */
    uint32_t oe_inv : 1;  /* Invert output enable. */
    uint32_t do_sel : 1;  /* Data output select: 0 for DO, 1 for OE. */
    uint32_t do_inv : 1;  /* Invert the result of data output select (DO_SEL). */
    uint32_t pu : 1;      /* Pull up enable. 0 for nothing, 1 for pull up. */
    uint32_t pd : 1;      /* Pull down enable. 0 for nothing, 1 for pull down. */
    uint32_t resv0 : 1;   /* Reserved bits. */
    uint32_t sl : 1;      /* Slew rate control enable. */
    uint32_t ie_en : 1;   /* Static input enable, will AND with IE_INV. */
    uint32_t ie_inv : 1;  /* Invert input enable. */
    uint32_t di_inv : 1;  /* Invert Data input. */
    uint32_t st : 1;      /* Schmitt trigger. */
    uint32_t resv1 : 7;   /* Reserved bits. */
    uint32_t pad_di : 1;  /* Read current IO's data input. */
} __attribute__((packed, aligned(4))) fpioa_io_config_t;

typedef enum _fpioa_pull
{
    FPIOA_PULL_NONE,      /* No Pull */
    FPIOA_PULL_DOWN,      /* Pull Down */
    FPIOA_PULL_UP,        /* Pull Up */
    FPIOA_PULL_MAX        /* Count of pull settings */
} fpioa_pull_t;


/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/
int k210_fpioa_get_io_by_function(uint8_t function);
int fpioa_set_io_pull(int number, fpioa_pull_t pull);
void k210_fpioa_config(uint32_t io, uint32_t ioflags);

#endif /* __ARCH_RISCV_SRC_K210_K210_FPIOA_H */
