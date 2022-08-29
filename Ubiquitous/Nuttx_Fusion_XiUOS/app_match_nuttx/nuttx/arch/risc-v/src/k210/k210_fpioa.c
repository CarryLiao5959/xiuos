/****************************************************************************
 * arch/risc-v/src/k210/k210_fpioa.c
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
* @file k210_fpioa.c
* @brief nuttx source code
*                https://github.com/apache/incubator-nuttx.git
* @version 10.3.0 
* @author AIIT XUOS Lab
* @date 2022-03-23
*/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <assert.h>
#include <debug.h>

#include "riscv_internal.h"
#include "k210_memorymap.h"
#include "k210_fpioa.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int k210_fpioa_get_io_by_function(uint8_t function)
{
  int index = 0;
  uint32_t RegValue = 0x0000;
  uint32_t *fpioa = (uint32_t *)K210_FPIOA_BASE;

  for (index = 0; index < K210_IO_NUMBER; index++)
  {
      RegValue = getreg32(&fpioa[index]);
      if ((RegValue & 0xFF) == function)
          return index;
  }
  return -1;
}

int fpioa_set_io_pull(int number, fpioa_pull_t pull)
{
  /* Check parameters */
  if (number < 0 || number >= K210_IO_NUMBER || pull >= FPIOA_PULL_MAX)
      return -1;

  /* read register */
  uint32_t *fpioa = (uint32_t *)K210_FPIOA_BASE;
  fpioa_io_config_t cfg = *(fpioa_io_config_t *)(&fpioa[number]);
  uint32_t ioflags = 0x0000;

  switch (pull)
  {
      case FPIOA_PULL_NONE:
          cfg.pu = 0;
          cfg.pd = 0;
          break;
      case FPIOA_PULL_DOWN:
          cfg.pu = 0;
          cfg.pd = 1;
          break;
      case FPIOA_PULL_UP:
          cfg.pu = 1;
          cfg.pd = 0;
          break;
      default:
          break;
  }

  /* write register */
  ioflags = *(uint32_t*)(&cfg);
  putreg32(ioflags, &fpioa[number]);
  return 0;
}

void k210_fpioa_config(uint32_t io, uint32_t ioflags)
{
  uint32_t *fpioa = (uint32_t *)K210_FPIOA_BASE;
  DEBUGASSERT(io < K210_IO_NUMBER);
  putreg32(ioflags, &fpioa[io]);
}
