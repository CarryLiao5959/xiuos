/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
 * @file k210_gpio.c
 * @brief xidatong-riscv64 k210_gpio.c
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2022.06.08
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <syslog.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/irq.h>
#include <arch/irq.h>

#include <nuttx/ioexpander/gpio.h>

#include <arch/board/board.h>

#include "k210_fpioa.h"
#include "k210_gpiohs.h"

#if defined(CONFIG_DEV_GPIO) && !defined(CONFIG_GPIO_LOWER_HALF)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Pin 1 and 2 are used for this example as GPIO outputs. */

#define GPIO_E220_M0   44
#define GPIO_E220_M1   45
#define GPIO_E18_MODE  46
#define GPIO_WIFI_EN   8

#define FPIOA_E220_M0   1
#define FPIOA_E220_M1   2
#define FPIOA_E18_MODE  3
#define FPIOA_WIFI_EN   4

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct k210gpio_dev_s
{
  struct gpio_dev_s gpio;
  uint8_t id;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

#if BOARD_NGPIOOUT > 0
static int gpout_read(FAR struct gpio_dev_s *dev, FAR bool *value);
static int gpout_write(FAR struct gpio_dev_s *dev, bool value);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

#if BOARD_NGPIOOUT > 0
static const struct gpio_operations_s gpout_ops =
{
  .go_read   = gpout_read,
  .go_write  = gpout_write,
  .go_attach = NULL,
  .go_enable = NULL,
};

/* This array maps the GPIO pins used as OUTPUT */

static const uint32_t g_gpiooutputs[BOARD_NGPIOOUT] =
{
  GPIO_E220_M0, 
  GPIO_E220_M1,
  GPIO_E18_MODE,
  GPIO_WIFI_EN
};

static const uint32_t g_fpioa[BOARD_NGPIOOUT] =
{
  FPIOA_E220_M0, 
  FPIOA_E220_M1,
  FPIOA_E18_MODE,
  FPIOA_WIFI_EN
};

static struct k210gpio_dev_s g_gpout[BOARD_NGPIOOUT];
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: gpout_read
 ****************************************************************************/

#if BOARD_NGPIOOUT > 0
static int gpout_read(FAR struct gpio_dev_s *dev, FAR bool *value)
{
  FAR struct k210gpio_dev_s *k210gpio =
    (FAR struct k210gpio_dev_s *)dev;

  DEBUGASSERT(k210gpio != NULL && value != NULL);
  DEBUGASSERT(k210gpio->id < BOARD_NGPIOOUT);
  gpioinfo("Reading...\n");

  *value = (int) k210_gpiohs_get_value(g_fpioa[k210gpio->id]);
  return OK;
}

/****************************************************************************
 * Name: gpout_write
 ****************************************************************************/

static int gpout_write(FAR struct gpio_dev_s *dev, bool value)
{
  FAR struct k210gpio_dev_s *k210gpio =
    (FAR struct k210gpio_dev_s *)dev;

  DEBUGASSERT(k210gpio != NULL);
  DEBUGASSERT(k210gpio->id < BOARD_NGPIOOUT);
  gpioinfo("Writing %d\n", (int)value);

  k210_gpiohs_set_value(g_fpioa[k210gpio->id], value);

  return OK;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: k210_gpio_init
 ****************************************************************************/

int k210_gpio_init(void)
{
  int i;
  int pincount = 0;

#if BOARD_NGPIOOUT > 0
  for (i = 0; i < BOARD_NGPIOOUT; i++)
    {
      /* Setup and register the GPIO pin */

      g_gpout[i].gpio.gp_pintype = GPIO_OUTPUT_PIN;
      g_gpout[i].gpio.gp_ops     = &gpout_ops;
      g_gpout[i].id              = i;
      gpio_pin_register(&g_gpout[i].gpio, pincount);

      /* Configure the pins that will be used as output */

      k210_fpioa_config(g_gpiooutputs[i],
                        (K210_IO_FUNC_GPIOHS0 + g_fpioa[i]) | K210_IOFLAG_GPIOHS);
      k210_gpiohs_set_direction(g_fpioa[i], GPIO_DM_OUTPUT);
      k210_gpiohs_set_value(g_fpioa[i], false);

      pincount++;
    }
#endif

  return OK;
}
#endif /* CONFIG_DEV_GPIO && !CONFIG_GPIO_LOWER_HALF */
