/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
 * @file k210_bringup.c
 * @brief xidatong-riscv64 k210_bringup.c
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2022.03.17
 */


/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/board.h>
#include <nuttx/fs/fs.h>

#include "k210.h"
#include "k210_clockconfig.h"
#include "xidatong-riscv64.h"

#ifdef CONFIG_BSP_USING_CH438
#  include "k210_ch438.h"
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: k210_bringup
 ****************************************************************************/

int k210_bringup(void)
{
  int ret = OK;

#ifdef CONFIG_FS_PROCFS
  /* Mount the procfs file system */

  ret = nx_mount(NULL, "/proc", "procfs", 0, NULL);
  if (ret < 0)
    {
      serr("ERROR: Failed to mount procfs at %s: %d\n", "/proc", ret);
    }
#endif

#ifdef CONFIG_DEV_GPIO
  ret = k210_gpio_init();
  if (ret < 0)
    {
      syslog(LOG_ERR, "Failed to initialize GPIO Driver: %d\n", ret);
      return ret;
    }
#endif

#ifdef CONFIG_BSP_USING_CH438
  board_ch438_initialize();
#endif

#ifdef CONFIG_K210_LCD
  k210_sysctl_init();
  ret = board_lcd_initialize();
  if (ret < 0)
    {
      syslog(LOG_NOTICE, "board lcd initialize %d\n", ret);
    }
#endif

  return ret;
}
