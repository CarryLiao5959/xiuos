/****************************************************************************
 * arch/risc-v/src/k210/k210_gpio.c
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
* @file k210_gpio.c
* @brief nuttx source code
*                https://github.com/apache/incubator-nuttx.git
* @version 10.3.0 
* @author AIIT XUOS Lab
* @date 2022-06-30
*/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <assert.h>
#include <debug.h>

#include "riscv_internal.h"
#include "k210_memorymap.h"
#include "k210_gpio.h"
#include "k210_fpioa.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define GPIO_INPUT_VAL_OFFSET    0x00
#define GPIO_INPUT_EN_OFFSET     0x04
#define GPIO_OUTPUT_EN_OFFSET    0x08
#define GPIO_OUTPUT_VAL_OFFSET   0x0c
#define GPIO_PULLUP_EN_OFFSET    0x10
#define GPIO_DRIVE_OFFSET        0x14

#define GPIO_INPUT      (K210_GPIO_BASE + GPIO_INPUT_VAL_OFFSET)
#define GPIO_INPUT_EN   (K210_GPIO_BASE + GPIO_INPUT_EN_OFFSET)
#define GPIO_OUTPUT     (K210_GPIO_BASE + GPIO_OUTPUT_VAL_OFFSET)
#define GPIO_OUTPUT_EN  (K210_GPIO_BASE + GPIO_OUTPUT_EN_OFFSET)

/****************************************************************************
 * Public Functions
 ****************************************************************************/
void k210_gpio_set_direction(uint32_t io, gpio_drive_mode_t mode)
{
  DEBUGASSERT(io < K210_GPIO_MAX_PINNO);
  int io_number = k210_fpioa_get_io_by_function(K210_IO_FUNC_GPIO0 + io);
  DEBUGASSERT(io_number >= 0);

  fpioa_pull_t pull = FPIOA_PULL_NONE;
  uint32_t dir = 0;

  switch (mode)
  {
      case GPIO_DM_INPUT:
          pull = FPIOA_PULL_NONE;
          dir = 0;
          break;
      case GPIO_DM_INPUT_PULL_DOWN:
          pull = FPIOA_PULL_DOWN;
          dir = 0;
          break;
      case GPIO_DM_INPUT_PULL_UP:
          pull = FPIOA_PULL_UP;
          dir = 0;
          break;
      case GPIO_DM_OUTPUT:
          pull = FPIOA_PULL_DOWN;
          dir = 1;
          break;
      default:
          DEBUGASSERT(!"GPIO drive mode is not supported.");
          break;
  }

  fpioa_set_io_pull(io_number, pull);
  uint32_t outbit = dir << io;
  uint32_t inbit  = (!dir) << io;
  modifyreg32(GPIO_OUTPUT_EN, inbit, outbit);
  modifyreg32(GPIO_INPUT_EN, outbit, inbit);
}

void k210_gpio_set_value(uint32_t io, bool val)
{
  uint32_t setbit = val << io;
  uint32_t clrbit  = (!val) << io;
  modifyreg32(GPIO_OUTPUT, clrbit, setbit);
}

bool k210_gpio_get_value(uint32_t io)
{
  uint32_t reg = getreg32(GPIO_INPUT);

  if (reg & (1 << io))
    {
      return true;
    }
  else
    {
      return false;
    }
}
