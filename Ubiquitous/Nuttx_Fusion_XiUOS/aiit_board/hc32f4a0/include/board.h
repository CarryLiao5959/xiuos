/****************************************************************************
 * boards/arm/hc32/hc32f4a0/include/board.h
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
* @file board.h
* @brief nuttx source code
*                https://github.com/apache/incubator-nuttx.git
* @version 10.3.0
* @author AIIT XUOS Lab
* @date 2022-10-10
*/

#ifndef __BOARDS_ARM_HC32_HC32F4A0_INCLUDE_BOARD_H
#define __BOARDS_ARM_HC32_HC32F4A0_INCLUDE_BOARD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <stdint.h>
#  include <stdbool.h>
#endif

/* Do not include HC32-specific header files here */

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LED_GREEN_PORT              (GPIO_PORT_I)
#define LED_GREEN_PIN               (GPIO_PIN_03)

/* Key port&pin definition */
#define KEY_PORT                    (GPIO_PORT_I)
#define KEY_PIN                     (GPIO_PIN_07)

/* for lowputc device output */

/* UART RX/TX Port/Pin definition */
#define LP_RX_PORT                   (GPIO_PORT_H)   /* PH6: USART6_RX */
#define LP_RX_PIN                    (GPIO_PIN_06)
#define LP_RX_GPIO_FUNC              (GPIO_FUNC_37_USART6_RX)

#define LP_TX_PORT                   (GPIO_PORT_E)   /* PE6: USART6_TX */
#define LP_TX_PIN                    (GPIO_PIN_06)
#define LP_TX_GPIO_FUNC              (GPIO_FUNC_36_USART6_TX)

/* UART unit definition */
#define LP_UNIT                      (M4_USART6)
#define LP_FUNCTION_CLK_GATE         (PWC_FCG3_USART6)

/* UART unit interrupt definition */
#define LP_UNIT_ERR_INT_SRC          (INT_USART6_EI)
#define LP_UNIT_ERR_INT_IRQn         (Int015_IRQn + HC32_IRQ_FIRST)

#define LP_UNIT_RX_INT_SRC           (INT_USART6_RI)
#define LP_UNIT_RX_INT_IRQn          (Int103_IRQn + HC32_IRQ_FIRST)

#define LP_UNIT_TX_INT_SRC           (INT_USART6_TI)
#define LP_UNIT_TX_INT_IRQn          (Int102_IRQn + HC32_IRQ_FIRST)

#define LP_UNIT_TCI_INT_SRC          (INT_USART6_TCI)
#define LP_UNIT_TCI_INT_IRQn         (Int099_IRQn + HC32_IRQ_FIRST)

/* printf device s*/
#define BSP_PRINTF_DEVICE            LP_UNIT
#define BSP_PRINTF_BAUDRATE          (115200)

#define BSP_PRINTF_PORT              LP_TX_PORT

#define BSP_PRINTF_PIN               LP_TX_PIN
#define BSP_PRINTF_PORT_FUNC         LP_TX_GPIO_FUNC

#endif /* __BOARDS_ARM_HC32_HC32F4A0_INCLUDE_BOARD_H */
