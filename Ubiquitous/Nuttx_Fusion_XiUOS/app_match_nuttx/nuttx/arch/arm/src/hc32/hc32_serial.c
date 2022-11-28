/****************************************************************************
 * arch/arm/src/hc32/hc32_serial.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/serial/serial.h>
#include <nuttx/power/pm.h>

#ifdef CONFIG_SERIAL_TERMIOS
#  include <termios.h>
#endif

#include <arch/board/board.h>

#include "chip.h"
#include "hc32_uart.h"
#include "hc32_rcc.h"
#include "hc32_gpio.h"
#include "arm_internal.h"
#include "hc32f4a0.h"
#include "hc32f4a0_usart.h"
#include "hc32f4a0_gpio.h"
#include "hc32f4a0_interrupts.h"
#include "hc32f4a0_sram.h"
#include "hc32f4a0_pwc.h"
#include "hc32f4a0_efm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define USART_CR1_USED_INTS    (USART_CR1_RIE | USART_CR1_TXEIE | USART_CR1_PCE | USART_CR1_TCIE)


/* Some sanity checks *******************************************************/

/* DMA configuration */

/* If DMA is enabled on any USART, then very that other pre-requisites
 * have also been selected.
 */

/* Power management definitions */

#if defined(CONFIG_PM) && !defined(CONFIG_HC32_PM_SERIAL_ACTIVITY)
#  define CONFIG_HC32_PM_SERIAL_ACTIVITY 10
#endif
#if defined(CONFIG_PM)
#  define PM_IDLE_DOMAIN             0 /* Revisit */
#endif

/* Since RX DMA or TX DMA or both may be enabled for a given U[S]ART.
 * We need runtime detection in up_dma_setup and up_dma_shutdown
 * We use the default struct default init value of 0 which maps to
 * HC32_DMA_MAP(DMA1,DMA_STREAM0,DMA_CHAN0) which is not a U[S]ART.
 */

#define INVALID_SERIAL_DMA_CHANNEL 0

/* Keep track if a Break was set
 *
 * Note:
 *
 * 1) This value is set in the priv->ie but never written to the control
 *    register. It must not collide with USART_CR1_USED_INTS or USART_CR3_EIE
 * 2) USART_CR3_EIE is also carried in the up_dev_s ie member.
 *
 * See up_restoreusartint where the masking is done.
 */

#ifdef CONFIG_HC32_SERIALBRK_BSDCOMPAT
#  define USART_CR1_IE_BREAK_INPROGRESS_SHFTS 15
#  define USART_CR1_IE_BREAK_INPROGRESS (1 << USART_CR1_IE_BREAK_INPROGRESS_SHFTS)
#endif

#ifdef USE_SERIALDRIVER
#ifdef HAVE_SERIALDRIVER

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct up_dev_s
{
  struct uart_dev_s dev;       /* Generic UART device */
  uint16_t          ie;        /* Saved interrupt mask bits value */
  uint16_t          sr;        /* Saved status bits */

  /* Has been initialized and HW is setup. */

  bool              initialized;

  /* If termios are supported, then the following fields may vary at
   * runtime.
   */

#ifdef CONFIG_SERIAL_TERMIOS
  uint8_t           parity;    /* 0=none, 1=odd, 2=even */
  uint8_t           bits;      /* Number of bits (7 or 8) */
  bool              stopbits2; /* True: Configure with 2 stop bits instead of 1 */
#ifdef CONFIG_SERIAL_IFLOWCONTROL
  bool              iflow;     /* input flow control (RTS) enabled */
#endif
#ifdef CONFIG_SERIAL_OFLOWCONTROL
  bool              oflow;     /* output flow control (CTS) enabled */
#endif
  uint32_t          baud;      /* Configured baud */
#else
  const uint8_t     parity;    /* 0=none, 1=odd, 2=even */
  const uint8_t     bits;      /* Number of bits (7 or 8) */
  const bool        stopbits2; /* True: Configure with 2 stop bits instead of 1 */
#ifdef CONFIG_SERIAL_IFLOWCONTROL
  const bool        iflow;     /* input flow control (RTS) enabled */
#endif
#ifdef CONFIG_SERIAL_OFLOWCONTROL
  const bool        oflow;     /* output flow control (CTS) enabled */
#endif
  const uint32_t    baud;      /* Configured baud */
#endif

  const uint32_t    irq;       /* IRQ associated with this USART */
  const uint32_t    apbclock;  /* PCLK 1 or 2 frequency */
  const uint32_t    usartbase; /* Base address of USART registers */
  const uint32_t    tx_gpio;   /* U[S]ART TX GPIO pin configuration */
  const uint32_t    rx_gpio;   /* U[S]ART RX GPIO pin configuration */
#ifdef CONFIG_SERIAL_IFLOWCONTROL
  const uint32_t    rts_gpio;  /* U[S]ART RTS GPIO pin configuration */
#endif
#ifdef CONFIG_SERIAL_OFLOWCONTROL
  const uint32_t    cts_gpio;  /* U[S]ART CTS GPIO pin configuration */
#endif

#ifdef HAVE_RS485
  const uint32_t    rs485_dir_gpio;     /* U[S]ART RS-485 DIR GPIO pin cfg */
  const bool        rs485_dir_polarity; /* U[S]ART RS-485 DIR TXEN polarity */
#endif
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void up_set_format(struct uart_dev_s *dev);
static int  up_setup(struct uart_dev_s *dev);
static void up_shutdown(struct uart_dev_s *dev);
static int  up_attach(struct uart_dev_s *dev);
static void up_detach(struct uart_dev_s *dev);
static int  up_interrupt(int irq, void *context, void *arg);
static int  up_ioctl(struct file *filep, int cmd, unsigned long arg);
static int  up_receive(struct uart_dev_s *dev, unsigned int *status);
static void up_rxint(struct uart_dev_s *dev, bool enable);
static bool up_rxavailable(struct uart_dev_s *dev);
#ifdef CONFIG_SERIAL_IFLOWCONTROL
static bool up_rxflowcontrol(struct uart_dev_s *dev, unsigned int nbuffered,
                             bool upper);
#endif
static void up_send(struct uart_dev_s *dev, int ch);
static void up_txint(struct uart_dev_s *dev, bool enable);
static bool up_txready(struct uart_dev_s *dev);


#ifdef CONFIG_PM
static void up_pm_notify(struct pm_callback_s *cb, int dowmin,
                         enum pm_state_e pmstate);
static int  up_pm_prepare(struct pm_callback_s *cb, int domain,
                          enum pm_state_e pmstate);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct uart_ops_s g_uart_ops =
{
  .setup          = up_setup,
  .shutdown       = up_shutdown,
  .attach         = up_attach,
  .detach         = up_detach,
  .ioctl          = up_ioctl,
  .receive        = up_receive,
  .rxint          = up_rxint,
  .rxavailable    = up_rxavailable,
#ifdef CONFIG_SERIAL_IFLOWCONTROL
  .rxflowcontrol  = up_rxflowcontrol,
#endif
  .send           = up_send,
  .txint          = up_txint,
  .txready        = up_txready,
  .txempty        = up_txready,
};

/* I/O buffers */

#ifdef CONFIG_UART1_SERIALDRIVER
static char g_usart1rxbuffer[CONFIG_USART1_RXBUFSIZE];
static char g_usart1txbuffer[CONFIG_USART1_TXBUFSIZE];
#endif

#ifdef CONFIG_UART2_SERIALDRIVER
static char g_usart2rxbuffer[CONFIG_USART2_RXBUFSIZE];
static char g_usart2txbuffer[CONFIG_USART2_TXBUFSIZE];
#endif

#ifdef CONFIG_UART3_SERIALDRIVER
static char g_usart3rxbuffer[CONFIG_USART3_RXBUFSIZE];
static char g_usart3txbuffer[CONFIG_USART3_TXBUFSIZE];
#endif

#ifdef CONFIG_UART4_SERIALDRIVER
static char g_uart4rxbuffer[CONFIG_UART4_RXBUFSIZE];
static char g_uart4txbuffer[CONFIG_UART4_TXBUFSIZE];
#endif

#ifdef CONFIG_UART5_SERIALDRIVER
static char g_uart5rxbuffer[CONFIG_UART5_RXBUFSIZE];
static char g_uart5txbuffer[CONFIG_UART5_TXBUFSIZE];
#endif

#ifdef CONFIG_UART6_SERIALDRIVER
static char g_usart6rxbuffer[CONFIG_UART6_RXBUFSIZE];
static char g_usart6txbuffer[CONFIG_UART6_TXBUFSIZE];
#endif

#ifdef CONFIG_UART7_SERIALDRIVER
static char g_uart7rxbuffer[CONFIG_UART7_RXBUFSIZE];
static char g_uart7txbuffer[CONFIG_UART7_TXBUFSIZE];
#endif

#ifdef CONFIG_UART8_SERIALDRIVER
static char g_uart8rxbuffer[CONFIG_UART8_RXBUFSIZE];
static char g_uart8txbuffer[CONFIG_UART8_TXBUFSIZE];
#endif

/* This describes the state of the HC32 USART1 ports. */

#ifdef CONFIG_UART1_SERIALDRIVER
static struct up_dev_s g_usart1priv =
{
  .dev =
    {
#if CONSOLE_UART == 1
      .isconsole = true,
#endif
      .recv      =
      {
        .size    = CONFIG_USART1_RXBUFSIZE,
        .buffer  = g_usart1rxbuffer,
      },
      .xmit      =
      {
        .size    = CONFIG_USART1_TXBUFSIZE,
        .buffer  = g_usart1txbuffer,
      },

      .ops       = &g_uart_ops,
      .priv      = &g_usart1priv,
    },

  .irq           = HC32_IRQ_USART1,
  .parity        = CONFIG_USART1_PARITY,
  .bits          = CONFIG_USART1_BITS,
  .stopbits2     = CONFIG_USART1_2STOP,
  .baud          = CONFIG_USART1_BAUD,
  .apbclock      = HC32_PCLK2_FREQUENCY,
  .usartbase     = HC32_UART1_BASE,
  .tx_gpio       = GPIO_USART1_TX,
  .rx_gpio       = GPIO_USART1_RX,
#if defined(CONFIG_SERIAL_OFLOWCONTROL) && defined(CONFIG_USART1_OFLOWCONTROL)
  .oflow         = true,
  .cts_gpio      = GPIO_USART1_CTS,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_USART1_IFLOWCONTROL)
  .iflow         = true,
  .rts_gpio      = GPIO_USART1_RTS,
#endif

#ifdef CONFIG_USART1_RS485
  .rs485_dir_gpio = GPIO_USART1_RS485_DIR,
#  if (CONFIG_USART1_RS485_DIR_POLARITY == 0)
  .rs485_dir_polarity = false,
#  else
  .rs485_dir_polarity = true,
#  endif
#endif
};
#endif

/* This describes the state of the HC32 USART2 port. */

#ifdef CONFIG_UART2_SERIALDRIVER
static struct up_dev_s g_usart2priv =
{
  .dev =
    {
#if CONSOLE_UART == 2
      .isconsole = true,
#endif
      .recv      =
      {
        .size    = CONFIG_USART2_RXBUFSIZE,
        .buffer  = g_usart2rxbuffer,
      },
      .xmit      =
      {
        .size    = CONFIG_USART2_TXBUFSIZE,
        .buffer  = g_usart2txbuffer,
      },

      .ops       = &g_uart_ops,
      .priv      = &g_usart2priv,
    },

  .irq           = HC32_IRQ_USART2,
  .parity        = CONFIG_USART2_PARITY,
  .bits          = CONFIG_USART2_BITS,
  .stopbits2     = CONFIG_USART2_2STOP,
  .baud          = CONFIG_USART2_BAUD,
  .apbclock      = HC32_PCLK1_FREQUENCY,
  .usartbase     = HC32_UART2_BASE,
  .tx_gpio       = GPIO_USART2_TX,
  .rx_gpio       = GPIO_USART2_RX,
#if defined(CONFIG_SERIAL_OFLOWCONTROL) && defined(CONFIG_USART2_OFLOWCONTROL)
  .oflow         = true,
  .cts_gpio      = GPIO_USART2_CTS,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_USART2_IFLOWCONTROL)
  .iflow         = true,
  .rts_gpio      = GPIO_USART2_RTS,
#endif

#ifdef CONFIG_USART2_RS485
  .rs485_dir_gpio = GPIO_USART2_RS485_DIR,
#  if (CONFIG_USART2_RS485_DIR_POLARITY == 0)
  .rs485_dir_polarity = false,
#  else
  .rs485_dir_polarity = true,
#  endif
#endif
};
#endif

/* This describes the state of the HC32 USART3 port. */

#ifdef CONFIG_UART3_SERIALDRIVER
static struct up_dev_s g_usart3priv =
{
  .dev =
    {
#if CONSOLE_UART == 3
      .isconsole = true,
#endif
      .recv      =
      {
        .size    = CONFIG_USART3_RXBUFSIZE,
        .buffer  = g_usart3rxbuffer,
      },
      .xmit      =
      {
        .size    = CONFIG_USART3_TXBUFSIZE,
        .buffer  = g_usart3txbuffer,
      },
#if defined(CONFIG_USART3_RXDMA) && defined(CONFIG_USART3_TXDMA)
      .ops       = &g_uart_rxtxdma_ops,
#elif defined(CONFIG_USART3_RXDMA) && !defined(CONFIG_USART3_TXDMA)
      .ops       = &g_uart_rxdma_ops,
#elif !defined(CONFIG_USART3_RXDMA) && defined(CONFIG_USART3_TXDMA)
      .ops       = &g_uart_txdma_ops,
#else
      .ops       = &g_uart_ops,
#endif
      .priv      = &g_usart3priv,
    },

  .irq           = HC32_IRQ_USART3,
  .parity        = CONFIG_USART3_PARITY,
  .bits          = CONFIG_USART3_BITS,
  .stopbits2     = CONFIG_USART3_2STOP,
  .baud          = CONFIG_USART3_BAUD,
  .apbclock      = HC32_PCLK1_FREQUENCY,
  .usartbase     = HC32_UART3_BASE,
  .tx_gpio       = GPIO_USART3_TX,
  .rx_gpio       = GPIO_USART3_RX,
#if defined(CONFIG_SERIAL_OFLOWCONTROL) && defined(CONFIG_USART3_OFLOWCONTROL)
  .oflow         = true,
  .cts_gpio      = GPIO_USART3_CTS,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_USART3_IFLOWCONTROL)
  .iflow         = true,
  .rts_gpio      = GPIO_USART3_RTS,
#endif

#ifdef CONFIG_USART3_RS485
  .rs485_dir_gpio = GPIO_USART3_RS485_DIR,
#  if (CONFIG_USART3_RS485_DIR_POLARITY == 0)
  .rs485_dir_polarity = false,
#  else
  .rs485_dir_polarity = true,
#  endif
#endif
};
#endif

/* This describes the state of the HC32 UART4 port. */

#ifdef CONFIG_UART4_SERIALDRIVER
static struct up_dev_s g_uart4priv =
{
  .dev =
    {
#if CONSOLE_UART == 4
      .isconsole = true,
#endif
      .recv      =
      {
        .size    = CONFIG_UART4_RXBUFSIZE,
        .buffer  = g_uart4rxbuffer,
      },
      .xmit      =
      {
        .size    = CONFIG_UART4_TXBUFSIZE,
        .buffer  = g_uart4txbuffer,
      },
#if defined(CONFIG_UART4_RXDMA) && defined(CONFIG_UART4_TXDMA)
      .ops       = &g_uart_rxtxdma_ops,
#elif defined(CONFIG_UART4_RXDMA) && !defined(CONFIG_UART4_TXDMA)
      .ops       = &g_uart_rxdma_ops,
#elif !defined(CONFIG_UART4_RXDMA) && defined(CONFIG_UART4_TXDMA)
      .ops       = &g_uart_txdma_ops,
#else
      .ops       = &g_uart_ops,
#endif
      .priv      = &g_uart4priv,
    },

  .irq           = HC32_IRQ_UART4,
  .parity        = CONFIG_UART4_PARITY,
  .bits          = CONFIG_UART4_BITS,
  .stopbits2     = CONFIG_UART4_2STOP,
#if defined(CONFIG_SERIAL_OFLOWCONTROL) && defined(CONFIG_UART4_OFLOWCONTROL)
  .oflow         = true,
  .cts_gpio      = GPIO_UART4_CTS,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_UART4_IFLOWCONTROL)
  .iflow         = true,
  .rts_gpio      = GPIO_UART4_RTS,
#endif
  .baud          = CONFIG_UART4_BAUD,
  .apbclock      = HC32_PCLK1_FREQUENCY,
  .usartbase     = HC32_UART4_BASE,
  .tx_gpio       = GPIO_UART4_TX,
  .rx_gpio       = GPIO_UART4_RX,

#ifdef CONFIG_UART4_RS485
  .rs485_dir_gpio = GPIO_UART4_RS485_DIR,
#  if (CONFIG_UART4_RS485_DIR_POLARITY == 0)
  .rs485_dir_polarity = false,
#  else
  .rs485_dir_polarity = true,
#  endif
#endif
};
#endif

/* This describes the state of the HC32 UART5 port. */

#ifdef CONFIG_UART5_SERIALDRIVER
static struct up_dev_s g_uart5priv =
{
  .dev =
    {
#if CONSOLE_UART == 5
      .isconsole = true,
#endif
      .recv     =
      {
        .size   = CONFIG_UART5_RXBUFSIZE,
        .buffer = g_uart5rxbuffer,
      },
      .xmit     =
      {
        .size   = CONFIG_UART5_TXBUFSIZE,
        .buffer = g_uart5txbuffer,
      },

      .ops       = &g_uart_ops,
      .priv     = &g_uart5priv,
    },

  .irq            = HC32_IRQ_UART5,
  .parity         = CONFIG_UART5_PARITY,
  .bits           = CONFIG_UART5_BITS,
  .stopbits2      = CONFIG_UART5_2STOP,
#if defined(CONFIG_SERIAL_OFLOWCONTROL) && defined(CONFIG_UART5_OFLOWCONTROL)
  .oflow         = true,
  .cts_gpio      = GPIO_UART5_CTS,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_UART5_IFLOWCONTROL)
  .iflow         = true,
  .rts_gpio      = GPIO_UART5_RTS,
#endif
  .baud           = CONFIG_UART5_BAUD,
  .apbclock       = HC32_PCLK1_FREQUENCY,
  .usartbase      = HC32_UART5_BASE,
  .tx_gpio        = GPIO_UART5_TX,
  .rx_gpio        = GPIO_UART5_RX,

#ifdef CONFIG_UART5_RS485
  .rs485_dir_gpio = GPIO_UART5_RS485_DIR,
#  if (CONFIG_UART5_RS485_DIR_POLARITY == 0)
  .rs485_dir_polarity = false,
#  else
  .rs485_dir_polarity = true,
#  endif
#endif
};
#endif

/* This describes the state of the HC32 USART6 port. */

#ifdef CONFIG_UART6_SERIALDRIVER

/* UART RX/TX Port/Pin definition */
#define USART_RX_PORT                   (GPIO_PORT_H)   /* PH6: USART6_RX */
#define USART_RX_PIN                    (GPIO_PIN_06)
#define USART_RX_GPIO_FUNC              (GPIO_FUNC_37_USART6_RX)

#define USART_TX_PORT                   (GPIO_PORT_E)   /* PE6: USART6_TX */
#define USART_TX_PIN                    (GPIO_PIN_06)
#define USART_TX_GPIO_FUNC              (GPIO_FUNC_36_USART6_TX)

/* UART unit definition */
#define USART_UNIT                      (M4_USART6)
#define USART_FUNCTION_CLK_GATE         (PWC_FCG3_USART6)

/* UART unit interrupt definition */
#define USART_UNIT_ERR_INT_SRC          (INT_USART6_EI)
#define USART_UNIT_ERR_INT_IRQn         (Int015_IRQn + HC32_IRQ_FIRST)

#define USART_UNIT_RX_INT_SRC           (INT_USART6_RI)
#define USART_UNIT_RX_INT_IRQn          (Int010_IRQn + HC32_IRQ_FIRST)

#define USART_UNIT_TX_INT_SRC           (INT_USART6_TI)
#define USART_UNIT_TX_INT_IRQn          (Int102_IRQn + HC32_IRQ_FIRST)

#define USART_UNIT_TCI_INT_SRC          (INT_USART6_TCI)
#define USART_UNIT_TCI_INT_IRQn         (Int099_IRQn + HC32_IRQ_FIRST)

#define HC32_UART6_BASE 0x40020C00UL// M4_USART6
#define HC32_IRQ_USART6 (Int010_IRQn + HC32_IRQ_FIRST)
#define GPIO_USART6_TX GPIO_PINSET(USART_TX_PORT, USART_TX_PIN)
#define GPIO_USART6_RX GPIO_PINSET(USART_RX_PORT, USART_RX_PIN)

#define HC32_PCLK2_FREQUENCY 120000000

static struct up_dev_s g_usart6priv =
{
  .dev =
    {
#if CONSOLE_UART == 6
      .isconsole = true,
#endif
      .recv     =
      {
        .size   = CONFIG_UART6_RXBUFSIZE,
        .buffer = g_usart6rxbuffer,
      },
      .xmit     =
      {
        .size   = CONFIG_UART6_TXBUFSIZE,
        .buffer = g_usart6txbuffer,
      },
      .ops       = &g_uart_ops,
      .priv     = &g_usart6priv,
    },

  .irq            = HC32_IRQ_USART6,
  .parity         = CONFIG_UART6_PARITY,
  .bits           = CONFIG_UART6_BITS,
  .stopbits2      = CONFIG_UART6_2STOP,
  .baud           = CONFIG_UART6_BAUD,
  .apbclock       = HC32_PCLK2_FREQUENCY,
  .usartbase      = HC32_UART6_BASE,
  .tx_gpio        = GPIO_USART6_TX,
  .rx_gpio        = GPIO_USART6_RX,
#if defined(CONFIG_SERIAL_OFLOWCONTROL) && defined(CONFIG_USART6_OFLOWCONTROL)
  .oflow          = true,
  .cts_gpio       = GPIO_USART6_CTS,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_USART6_IFLOWCONTROL)
  .iflow          = true,
  .rts_gpio       = GPIO_USART6_RTS,
#endif

#ifdef CONFIG_USART6_RS485
  .rs485_dir_gpio = GPIO_USART6_RS485_DIR,
#  if (CONFIG_USART6_RS485_DIR_POLARITY == 0)
  .rs485_dir_polarity = false,
#  else
  .rs485_dir_polarity = true,
#  endif
#endif
};
#endif

/* This describes the state of the HC32 UART7 port. */

#ifdef CONFIG_UART7_SERIALDRIVER
static struct up_dev_s g_uart7priv =
{
  .dev =
    {
#if CONSOLE_UART == 7
      .isconsole = true,
#endif
      .recv     =
      {
        .size   = CONFIG_UART7_RXBUFSIZE,
        .buffer = g_uart7rxbuffer,
      },
      .xmit     =
      {
        .size   = CONFIG_UART7_TXBUFSIZE,
        .buffer = g_uart7txbuffer,
      },

      .ops       = &g_uart_ops,
      .priv     = &g_uart7priv,
    },

  .irq            = HC32_IRQ_UART7,
  .parity         = CONFIG_UART7_PARITY,
  .bits           = CONFIG_UART7_BITS,
  .stopbits2      = CONFIG_UART7_2STOP,
  .baud           = CONFIG_UART7_BAUD,
  .apbclock       = HC32_PCLK1_FREQUENCY,
  .usartbase      = HC32_UART7_BASE,
  .tx_gpio        = GPIO_UART7_TX,
  .rx_gpio        = GPIO_UART7_RX,
#if defined(CONFIG_SERIAL_OFLOWCONTROL) && defined(CONFIG_UART7_OFLOWCONTROL)
  .oflow          = true,
  .cts_gpio       = GPIO_UART7_CTS,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_UART7_IFLOWCONTROL)
  .iflow          = true,
  .rts_gpio       = GPIO_UART7_RTS,
#endif

#ifdef CONFIG_UART7_RS485
  .rs485_dir_gpio = GPIO_UART7_RS485_DIR,
#  if (CONFIG_UART7_RS485_DIR_POLARITY == 0)
  .rs485_dir_polarity = false,
#  else
  .rs485_dir_polarity = true,
#  endif
#endif
};
#endif

/* This describes the state of the HC32 UART8 port. */

#ifdef CONFIG_UART8_SERIALDRIVER
static struct up_dev_s g_uart8priv =
{
  .dev =
    {
#if CONSOLE_UART == 8
      .isconsole = true,
#endif
      .recv     =
      {
        .size   = CONFIG_UART8_RXBUFSIZE,
        .buffer = g_uart8rxbuffer,
      },
      .xmit     =
      {
        .size   = CONFIG_UART8_TXBUFSIZE,
        .buffer = g_uart8txbuffer,
      },
      .ops       = &g_uart_ops,
      .priv     = &g_uart8priv,
    },

  .irq            = HC32_IRQ_UART8,
  .parity         = CONFIG_UART8_PARITY,
  .bits           = CONFIG_UART8_BITS,
  .stopbits2      = CONFIG_UART8_2STOP,
  .baud           = CONFIG_UART8_BAUD,
  .apbclock       = HC32_PCLK1_FREQUENCY,
  .usartbase      = HC32_UART8_BASE,
  .tx_gpio        = GPIO_UART8_TX,
  .rx_gpio        = GPIO_UART8_RX,
#if defined(CONFIG_SERIAL_OFLOWCONTROL) && defined(CONFIG_UART8_OFLOWCONTROL)
  .oflow          = true,
  .cts_gpio       = GPIO_UART8_CTS,
#endif
#if defined(CONFIG_SERIAL_IFLOWCONTROL) && defined(CONFIG_UART8_IFLOWCONTROL)
  .iflow          = true,
  .rts_gpio       = GPIO_UART8_RTS,
#endif

#ifdef CONFIG_UART8_RS485
  .rs485_dir_gpio = GPIO_UART8_RS485_DIR,
#  if (CONFIG_UART8_RS485_DIR_POLARITY == 0)
  .rs485_dir_polarity = false,
#  else
  .rs485_dir_polarity = true,
#  endif
#endif
};
#endif

/* This table lets us iterate over the configured USARTs */

static struct up_dev_s * const g_uart_devs[HC32_NUSART] =
{
#ifdef CONFIG_UART1_SERIALDRIVER
  [0] = &g_usart1priv,
#endif
#ifdef CONFIG_UART2_SERIALDRIVER
  [1] = &g_usart2priv,
#endif
#ifdef CONFIG_UART3_SERIALDRIVER
  [2] = &g_usart3priv,
#endif
#ifdef CONFIG_UART4_SERIALDRIVER
  [3] = &g_uart4priv,
#endif
#ifdef CONFIG_UART5_SERIALDRIVER
  [4] = &g_uart5priv,
#endif
#ifdef CONFIG_UART6_SERIALDRIVER
  [5] = &g_usart6priv,
#endif
#ifdef CONFIG_UART7_SERIALDRIVER
  [6] = &g_uart7priv,
#endif
#ifdef CONFIG_UART8_SERIALDRIVER
  [7] = &g_uart8priv,
#endif
};

#ifdef CONFIG_PM
static  struct pm_callback_s g_serialcb =
{
  .notify  = up_pm_notify,
  .prepare = up_pm_prepare,
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

void hc32_print(const char *fmt, ...)
{
    int i;
    va_list ap;
    char buf[256];

    memset(buf, 0, sizeof(buf));
    va_start(ap, fmt);
    vsnprintf(buf, 255, fmt, ap);
    va_end(ap);

    USART_FuncCmd(USART_UNIT, USART_TX, Enable);

    for (i = 0; i < strlen(buf); i++)
    {
        while(Set != USART_GetStatus(USART_UNIT, USART_FLAG_TXE));
        USART_SendData(USART_UNIT, (uint16_t)*(buf + i));
    }
}

/****************************************************************************
 * Name: up_serialin
 ****************************************************************************/

static inline uint32_t up_serialin(struct up_dev_s *priv, int offset)
{
  return getreg32(priv->usartbase + offset);
}

/****************************************************************************
 * Name: up_serialout
 ****************************************************************************/

static inline void up_serialout(struct up_dev_s *priv, int offset,
                                uint32_t value)
{
  putreg32(value, priv->usartbase + offset);
}

/****************************************************************************
 * Name: up_setusartint
 ****************************************************************************/

static inline void up_setusartint(struct up_dev_s *priv, uint16_t ie)
{
  uint32_t cr;
  M4_USART_TypeDef *base = (M4_USART_TypeDef *)priv->usartbase;

  /* Save the interrupt mask */

  priv->ie = ie;

  /* And restore the interrupt state (see the interrupt enable/usage
   * table above)
   */

  cr  = up_serialin(priv, base->_CR1);
  cr &= ~(USART_CR1_USED_INTS);
  cr |= (ie & (USART_CR1_USED_INTS));
  up_serialout(priv, base->_CR1, cr);
//
//  cr  = up_serialin(priv, base->_CR3);
//  cr &= ~USART_CR3_EIE;
//  cr |= (ie & USART_CR3_EIE);
//  up_serialout(priv, base->_CR3, cr);
}

/****************************************************************************
 * Name: up_restoreusartint
 ****************************************************************************/

static void up_restoreusartint(struct up_dev_s *priv, uint16_t ie)
{
  irqstate_t flags;

//  hc32_print("check %s line %d\n", __func__, __LINE__);

  flags = enter_critical_section();

  up_setusartint(priv, ie);

  leave_critical_section(flags);
}

/****************************************************************************
 * Name: up_disableusartint
 ****************************************************************************/

static void up_disableusartint(struct up_dev_s *priv, uint16_t *ie)
{
  irqstate_t flags;
  M4_USART_TypeDef *base = (M4_USART_TypeDef *)priv->usartbase;

  flags = enter_critical_section();

  if (ie)
    {
      uint32_t cr1;

      /* USART interrupts:
       *
       * Enable             Status          Meaning                Usage
       * ------------------ --------------- ---------------------- ----------
       * USART_CR1_IDLEIE   USART_SR_IDLE   Idle Line Detected     (not used)
       * USART_CR1_RXNEIE   USART_SR_RXNE   Rx Data Ready
       * "              "   USART_SR_ORE    Overrun Error Detected
       * USART_CR1_TCIE     USART_SR_TC     Transmission Complete  (RS-485)
       * USART_CR1_TXEIE    USART_SR_TXE    Tx Data Register Empty
       * USART_CR1_PEIE     USART_SR_PE     Parity Error
       *
       * USART_CR2_LBDIE    USART_SR_LBD    Break Flag             (not used)
       * USART_CR3_EIE      USART_SR_FE     Framing Error
       * "           "      USART_SR_NE     Noise Error
       * "           "      USART_SR_ORE    Overrun Error Detected
       * USART_CR3_CTSIE    USART_SR_CTS    CTS flag               (not used)
       */

      cr1 = up_serialin(priv, base->_CR1);
      *ie = cr1 & (USART_CR1_USED_INTS);
    }

  /* Disable all interrupts */

  up_setusartint(priv, 0);

  leave_critical_section(flags);
}

/****************************************************************************
 * Name: up_set_format
 *
 * Description:
 *   Set the serial line format and speed.
 *
 ****************************************************************************/

#ifndef CONFIG_SUPPRESS_UART_CONFIG
static void up_set_format(struct uart_dev_s *dev)
{

}
#endif /* CONFIG_SUPPRESS_UART_CONFIG */

/****************************************************************************
 * Name: up_set_apb_clock
 *
 * Description:
 *   Enable or disable APB clock for the USART peripheral
 *
 * Input Parameters:
 *   dev - A reference to the UART driver state structure
 *   on  - Enable clock if 'on' is 'true' and disable if 'false'
 *
 ****************************************************************************/

static void up_set_apb_clock(struct uart_dev_s *dev, bool on)
{

}

/****************************************************************************
 * Name: up_setup
 *
 * Description:
 *   Configure the USART baud, bits, parity, etc. This method is called the
 *   first time that the serial port is opened.
 *
 ****************************************************************************/

static int up_setup(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  M4_USART_TypeDef *base = (M4_USART_TypeDef *)priv->usartbase;

  hc32_print("%s irq %d\n", __func__, priv->irq);

#ifndef CONFIG_SUPPRESS_UART_CONFIG
  uint32_t regval;

  /* Note: The logic here depends on the fact that that the USART module
   * was enabled in hc32_lowsetup().
   */

  /* Enable USART APB1/2 clock */

  up_set_apb_clock(dev, true);

  /* Configure pins for USART use */

  hc32_configgpio(priv->tx_gpio);
  hc32_configgpio(priv->rx_gpio);

#ifdef CONFIG_SERIAL_OFLOWCONTROL
  if (priv->cts_gpio != 0)
    {
      hc32_configgpio(priv->cts_gpio);
    }
#endif

#ifdef CONFIG_SERIAL_IFLOWCONTROL
  if (priv->rts_gpio != 0)
    {
      uint32_t config = priv->rts_gpio;

#ifdef CONFIG_HC32_FLOWCONTROL_BROKEN
      /* Instead of letting hw manage this pin, we will bitbang */

      config = (config & ~GPIO_MODE_MASK) | GPIO_OUTPUT;
#endif
      hc32_configgpio(config);
    }
#endif

#ifdef HAVE_RS485
  if (priv->rs485_dir_gpio != 0)
    {
      hc32_configgpio(priv->rs485_dir_gpio);
      hc32_gpiowrite(priv->rs485_dir_gpio, !priv->rs485_dir_polarity);
    }
#endif

  /* Configure CR2
   * Clear STOP, CLKEN, CPOL, CPHA, LBCL, and interrupt enable bits
   */

  regval  = up_serialin(priv, base->_CR2);

  regval &= ~(USART_CR2_STOP | USART_CR2_CLKC | USART_CR2_WKUPE |
              USART_CR2_SBKL | USART_CR2_LBDL | USART_CR2_LBDIE);

  /* Configure STOP bits */

  if (priv->stopbits2)
    {
      regval |= USART_CR2_STOP;
    }

  up_serialout(priv, base->_CR2, regval);

  /* Configure CR1
   * Clear TE, REm and all interrupt enable bits
   */

  regval  = up_serialin(priv, base->_CR1);
  regval &= ~(USART_CR1_TE | USART_CR1_RE | USART_CR1_USED_INTS);

  up_serialout(priv, base->_CR1, regval);

  /* Configure CR3
   * Clear CTSE, RTSE, and all interrupt enable bits
   */

  regval  = up_serialin(priv, base->_CR3);
  regval &= ~(USART_CR3_CTSE | USART_CR3_RTSE);

  up_serialout(priv, base->_CR3, regval);

  /* Configure the USART line format and speed. */

  up_set_format(dev);

  /* Enable Rx, Tx, and the USART */

  regval      = up_serialin(priv, base->_CR1);
  regval     |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_USED_INTS);

  up_serialout(priv, base->_CR1, regval);

#endif /* CONFIG_SUPPRESS_UART_CONFIG */

  /* Set up the cached interrupt enables value */

  priv->ie    = 0;

  /* Mark device as initialized. */

  priv->initialized = true;

  return OK;
}

/****************************************************************************
 * Name: up_shutdown
 *
 * Description:
 *   Disable the USART.  This method is called when the serial
 *   port is closed
 *
 ****************************************************************************/

static void up_shutdown(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  M4_USART_TypeDef *base = (M4_USART_TypeDef *)priv->usartbase;
  uint32_t regval;

  hc32_print("check %s line %d\n", __func__, __LINE__);

  /* Mark device as uninitialized. */

  priv->initialized = false;

  /* Disable all interrupts */

  up_disableusartint(priv, NULL);

  /* Disable USART APB1/2 clock */

  up_set_apb_clock(dev, false);

  /* Disable Rx, Tx, and the UART */

  regval  = up_serialin(priv, base->_CR1);
  regval &= ~( USART_CR1_TE | USART_CR1_RE);
  up_serialout(priv, base->_CR1, regval);

  /* Release pins. "If the serial-attached device is powered down, the TX
   * pin causes back-powering, potentially confusing the device to the point
   * of complete lock-up."
   *
   * REVISIT:  Is unconfiguring the pins appropriate for all device?  If not,
   * then this may need to be a configuration option.
   */

  hc32_unconfiggpio(priv->tx_gpio);
  hc32_unconfiggpio(priv->rx_gpio);

#ifdef CONFIG_SERIAL_OFLOWCONTROL
  if (priv->cts_gpio != 0)
    {
      hc32_unconfiggpio(priv->cts_gpio);
    }
#endif

#ifdef CONFIG_SERIAL_IFLOWCONTROL
  if (priv->rts_gpio != 0)
    {
      hc32_unconfiggpio(priv->rts_gpio);
    }
#endif

#ifdef HAVE_RS485
  if (priv->rs485_dir_gpio != 0)
    {
      hc32_unconfiggpio(priv->rs485_dir_gpio);
    }
#endif
}

/****************************************************************************
 * Name: up_attach
 *
 * Description:
 *   Configure the USART to operation in interrupt driven mode.  This method
 *   is called when the serial port is opened.  Normally, this is just after
 *   the setup() method is called, however, the serial console may operate
 *   in a non-interrupt driven mode during the boot phase.
 *
 *   RX and TX interrupts are not enabled when by the attach method (unless
 *   the hardware supports multiple levels of interrupt enabling).  The RX
 *   and TX interrupts are not enabled until the txint() and rxint() methods
 *   are called.
 *
 ****************************************************************************/

static int up_attach(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  int ret;

  /* Attach and enable the IRQ */

  ret = irq_attach(priv->irq, up_interrupt, priv);
  if (ret == OK)
    {
      /* Enable the interrupt (RX and TX interrupts are still disabled
       * in the USART
       */

      up_enable_irq(priv->irq);
    }

  return ret;
}

/****************************************************************************
 * Name: up_detach
 *
 * Description:
 *   Detach USART interrupts.  This method is called when the serial port is
 *   closed normally just before the shutdown method is called.  The
 *   exception is the serial console which is never shutdown.
 *
 ****************************************************************************/

static void up_detach(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  up_disable_irq(priv->irq);
  irq_detach(priv->irq);

  hc32_print("check %s line %d irq %d\n", __func__, __LINE__, priv->irq);
}

/****************************************************************************
 * Name: up_interrupt
 *
 * Description:
 *   This is the USART interrupt handler.  It will be invoked when an
 *   interrupt received on the 'irq'  It should call uart_transmitchars or
 *   uart_receivechar to perform the appropriate data transfers.  The
 *   interrupt handling logic must be able to map the 'irq' number into the
 *   appropriate uart_dev_s structure in order to call these functions.
 *
 ****************************************************************************/

static int up_interrupt(int irq, void *context, void *arg)
{
  struct up_dev_s *priv = (struct up_dev_s *)arg;
  M4_USART_TypeDef *base = (M4_USART_TypeDef *)priv->usartbase;
  int  passes;
  bool handled;

  hc32_print("check %s irq %d come\n", __func__, irq);

  DEBUGASSERT(priv != NULL);

  /* Report serial activity to the power management logic */

#if defined(CONFIG_PM) && CONFIG_HC32_PM_SERIAL_ACTIVITY > 0
  pm_activity(PM_IDLE_DOMAIN, CONFIG_HC32_PM_SERIAL_ACTIVITY);
#endif

  /* Loop until there are no characters to be transferred or,
   * until we have been looping for a long time.
   */

  handled = true;
  for (passes = 0; passes < 256 && handled; passes++)
    {
      handled = false;

      /* Get the masked USART status word. */

      priv->sr = up_serialin(priv, base->SR);

      /* USART interrupts:
       *
       * Enable             Status          Meaning                Usage
       * ------------------ --------------- ---------------------- ----------
       * USART_CR1_IDLEIE   USART_SR_IDLE   Idle Line Detected     (not used)
       * USART_CR1_RXNEIE   USART_SR_RXNE   Rx Data Ready
       * "              "   USART_SR_ORE    Overrun Error Detected
       * USART_CR1_TCIE     USART_SR_TC     Tx Complete            (RS-485)
       * USART_CR1_TXEIE    USART_SR_TXE    Tx Data Register Empty
       * USART_CR1_PEIE     USART_SR_PE     Parity Error
       *
       * USART_CR2_LBDIE    USART_SR_LBD    Break Flag             (not used)
       * USART_CR3_EIE      USART_SR_FE     Framing Error
       * "           "      USART_SR_NE     Noise Error
       * "           "      USART_SR_ORE    Overrun Error Detected
       * USART_CR3_CTSIE    USART_SR_CTS    CTS flag               (not used)
       *
       * NOTE: Some of these status bits must be cleared by explicitly
       * writing zero to the SR register: USART_SR_CTS, USART_SR_LBD. Note of
       * those are currently being used.
       */

#ifdef HAVE_RS485
      /* Transmission of whole buffer is over - TC is set, TXEIE is cleared.
       * Note - this should be first, to have the most recent TC bit value
       * from SR register - sending data affects TC, but without refresh we
       * will not know that...
       */

      if (((priv->sr & USART_SR_TC) != 0) &&
          ((priv->ie & USART_CR1_TCIE) != 0) &&
          ((priv->ie & USART_CR1_TXEIE) == 0))
        {
          hc32_gpiowrite(priv->rs485_dir_gpio, !priv->rs485_dir_polarity);
          up_restoreusartint(priv, priv->ie & ~USART_CR1_TCIE);
        }
#endif

      /* Handle incoming, receive bytes. */

      if (((priv->sr & USART_SR_RXNE) != 0) &&
          ((priv->ie & USART_CR1_RIE) != 0))
        {
          /* Received data ready... process incoming bytes.  NOTE the check
           * for RXNEIE:  We cannot call uart_recvchards of RX interrupts are
           * disabled.
           */

          uart_recvchars(&priv->dev);
          handled = true;
        }

      /* We may still have to read from the DR register to clear any pending
       * error conditions.
       */

      else if ((priv->sr & (USART_SR_ORE | USART_SR_RXNE | USART_SR_FE)) != 0)
        {
#if defined(CONFIG_HC32_HC32F30XX) || defined(CONFIG_HC32_HC32F33XX) || \
    defined(CONFIG_HC32_HC32F37XX) || defined(CONFIG_HC32_HC32G4XXX)
          /* These errors are cleared by writing the corresponding bit to the
           * interrupt clear register (ICR).
           */

          up_serialout(priv, HC32_UART_ICR_OFFSET,
                      (USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_FECF));
#else
          /* If an error occurs, read from DR to clear the error (data has
           * been lost).  If ORE is set along with RXNE then it tells you
           * that the byte *after* the one in the data register has been
           * lost, but the data register value is correct.  That case will
           * be handled above if interrupts are enabled. Otherwise, that
           * good byte will be lost.
           */

          up_serialin(priv, base->DR); //RDR
#endif
        }

      /* Handle outgoing, transmit bytes */

      if (((priv->sr & USART_SR_TXE) != 0) &&
          ((priv->ie & USART_CR1_TXEIE) != 0))
        {
          /* Transmit data register empty ... process outgoing bytes */

          uart_xmitchars(&priv->dev);
          handled = true;
        }
    }

  return OK;
}

/****************************************************************************
 * Name: up_ioctl
 *
 * Description:
 *   All ioctl calls will be routed through this method
 *
 ****************************************************************************/

static int up_ioctl(struct file *filep, int cmd, unsigned long arg)
{
#if defined(CONFIG_SERIAL_TERMIOS) || defined(CONFIG_SERIAL_TIOCSERGSTRUCT) \
    || defined(CONFIG_HC32_SERIALBRK_BSDCOMPAT)
  struct inode      *inode = filep->f_inode;
  struct uart_dev_s *dev   = inode->i_private;
#endif
#if defined(CONFIG_SERIAL_TERMIOS) || defined(CONFIG_HC32_SERIALBRK_BSDCOMPAT)
  struct up_dev_s   *priv  = (struct up_dev_s *)dev->priv;
  M4_USART_TypeDef *base = (M4_USART_TypeDef *)priv->usartbase;
#endif
  int                ret   = OK;

  hc32_print("check %s line %d\n", __func__, __LINE__);

  switch (cmd)
    {
#ifdef CONFIG_SERIAL_TIOCSERGSTRUCT
    case TIOCSERGSTRUCT:
      {
        struct up_dev_s *user = (struct up_dev_s *)arg;
        if (!user)
          {
            ret = -EINVAL;
          }
        else
          {
            memcpy(user, dev, sizeof(struct up_dev_s));
          }
      }
      break;
#endif

#ifdef CONFIG_UART_SINGLEWIRE
    case TIOCSSINGLEWIRE:
      {
        /* Change the TX port to be open-drain/push-pull and enable/disable
         * half-duplex mode.
         */

        uint32_t cr = up_serialin(priv, base->_CR3);

#if defined(CONFIG_HC32_HC32F10XX)
        if ((arg & SER_SINGLEWIRE_ENABLED) != 0)
          {
            hc32_configgpio((priv->tx_gpio & ~(GPIO_CNF_MASK)) |
                             GPIO_CNF_AFOD);
            cr |= USART_CR3_HDSEL;
          }
        else
          {
            hc32_configgpio((priv->tx_gpio & ~(GPIO_CNF_MASK)) |
                             GPIO_CNF_AFPP);
            cr &= ~USART_CR3_HDSEL;
          }
#else
        if ((arg & SER_SINGLEWIRE_ENABLED) != 0)
          {
            uint32_t gpio_val = (arg & SER_SINGLEWIRE_PUSHPULL) ==
                                 SER_SINGLEWIRE_PUSHPULL ?
                                 GPIO_PUSHPULL : GPIO_OPENDRAIN;
            gpio_val |= ((arg & SER_SINGLEWIRE_PULL_MASK) ==
                         SER_SINGLEWIRE_PULLUP) ? GPIO_PULLUP
                                                : GPIO_FLOAT;
            gpio_val |= ((arg & SER_SINGLEWIRE_PULL_MASK) ==
                         SER_SINGLEWIRE_PULLDOWN) ? GPIO_PULLDOWN
                                                  : GPIO_FLOAT;
            hc32_configgpio((priv->tx_gpio & ~(GPIO_PUPD_MASK |
                                                GPIO_OPENDRAIN)) |
                             gpio_val);
            cr |= USART_CR3_HDSEL;
          }
        else
          {
            hc32_configgpio((priv->tx_gpio & ~(GPIO_PUPD_MASK |
                                                GPIO_OPENDRAIN)) |
                             GPIO_PUSHPULL);
            cr &= ~USART_CR3_HDSEL;
          }
#endif

        up_serialout(priv, base->_CR3, cr);
      }
     break;
#endif

#ifdef CONFIG_SERIAL_TERMIOS
    case TCGETS:
      {
        struct termios *termiosp = (struct termios *)arg;

        if (!termiosp)
          {
            ret = -EINVAL;
            break;
          }

        /* Note that since we only support 8/9 bit modes and
         * there is no way to report 9-bit mode, we always claim 8.
         */

        termiosp->c_cflag =
          ((priv->parity != 0) ? PARENB : 0) |
          ((priv->parity == 1) ? PARODD : 0) |
          ((priv->stopbits2) ? CSTOPB : 0) |
#ifdef CONFIG_SERIAL_OFLOWCONTROL
          ((priv->oflow) ? CCTS_OFLOW : 0) |
#endif
#ifdef CONFIG_SERIAL_IFLOWCONTROL
          ((priv->iflow) ? CRTS_IFLOW : 0) |
#endif
          CS8;

        cfsetispeed(termiosp, priv->baud);

        /* TODO: CCTS_IFLOW, CCTS_OFLOW */
      }
      break;

    case TCSETS:
      {
        struct termios *termiosp = (struct termios *)arg;

        if (!termiosp)
          {
            ret = -EINVAL;
            break;
          }

        /* Perform some sanity checks before accepting any changes */

        if (((termiosp->c_cflag & CSIZE) != CS8)
#ifdef CONFIG_SERIAL_OFLOWCONTROL
            || ((termiosp->c_cflag & CCTS_OFLOW) && (priv->cts_gpio == 0))
#endif
#ifdef CONFIG_SERIAL_IFLOWCONTROL
            || ((termiosp->c_cflag & CRTS_IFLOW) && (priv->rts_gpio == 0))
#endif
           )
          {
            ret = -EINVAL;
            break;
          }

        if (termiosp->c_cflag & PARENB)
          {
            priv->parity = (termiosp->c_cflag & PARODD) ? 1 : 2;
          }
        else
          {
            priv->parity = 0;
          }

        priv->stopbits2 = (termiosp->c_cflag & CSTOPB) != 0;
#ifdef CONFIG_SERIAL_OFLOWCONTROL
        priv->oflow = (termiosp->c_cflag & CCTS_OFLOW) != 0;
#endif
#ifdef CONFIG_SERIAL_IFLOWCONTROL
        priv->iflow = (termiosp->c_cflag & CRTS_IFLOW) != 0;
#endif

        /* Note that since there is no way to request 9-bit mode
         * and no way to support 5/6/7-bit modes, we ignore them
         * all here.
         */

        /* Note that only cfgetispeed is used because we have knowledge
         * that only one speed is supported.
         */

        priv->baud = cfgetispeed(termiosp);

        /* Effect the changes immediately - note that we do not implement
         * TCSADRAIN / TCSAFLUSH
         */

        up_set_format(dev);
      }
      break;
#endif /* CONFIG_SERIAL_TERMIOS */

#ifdef CONFIG_UART_BREAKS
#  ifdef CONFIG_HC32_SERIALBRK_BSDCOMPAT
    case TIOCSBRK:  /* BSD compatibility: Turn break on, unconditionally */
      {
        irqstate_t flags;
        uint32_t tx_break;

        flags = enter_critical_section();

        /* Disable any further tx activity */

        priv->ie |= USART_CR1_IE_BREAK_INPROGRESS;

        up_txint(dev, false);

        /* Configure TX as a GPIO output pin and Send a break signal */

        tx_break = GPIO_OUTPUT | (~(GPIO_MODE_MASK | GPIO_OUTPUT_SET) &
                                  priv->tx_gpio);
        hc32_configgpio(tx_break);

        leave_critical_section(flags);
      }
      break;

    case TIOCCBRK:  /* BSD compatibility: Turn break off, unconditionally */
      {
        irqstate_t flags;

        flags = enter_critical_section();

        /* Configure TX back to U(S)ART */

        hc32_configgpio(priv->tx_gpio);

        priv->ie &= ~USART_CR1_IE_BREAK_INPROGRESS;

        /* Enable further tx activity */

        up_txint(dev, true);

        leave_critical_section(flags);
      }
      break;
#  else
    case TIOCSBRK:  /* No BSD compatibility: Turn break on for M bit times */
      {
        uint32_t cr1;
        irqstate_t flags;

        flags = enter_critical_section();
        cr1   = up_serialin(priv, base->_CR1);
        up_serialout(priv, base->_CR1, cr1 | USART_CR1_SBK);
        leave_critical_section(flags);
      }
      break;

    case TIOCCBRK:  /* No BSD compatibility: May turn off break too soon */
      {
        uint32_t cr1;
        irqstate_t flags;

        flags = enter_critical_section();
        cr1   = up_serialin(priv, base->_CR1);
        up_serialout(priv, base->_CR1, cr1 & ~USART_CR1_SBK);
        leave_critical_section(flags);
      }
      break;
#  endif
#endif

    default:
      ret = -ENOTTY;
      break;
    }

  return ret;
}

/****************************************************************************
 * Name: up_receive
 *
 * Description:
 *   Called (usually) from the interrupt level to receive one
 *   character from the USART.  Error bits associated with the
 *   receipt are provided in the return 'status'.
 *
 ****************************************************************************/

static int up_receive(struct uart_dev_s *dev, unsigned int *status)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  M4_USART_TypeDef *base = (M4_USART_TypeDef *)priv->usartbase;

  return USART_RecData(base);
}

/****************************************************************************
 * Name: up_rxint
 *
 * Description:
 *   Call to enable or disable RX interrupts
 *
 ****************************************************************************/

static void up_rxint(struct uart_dev_s *dev, bool enable)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  irqstate_t flags;
  uint16_t ie;

  /* USART receive interrupts:
   *
   * Enable             Status          Meaning                   Usage
   * ------------------ --------------- ------------------------- ----------
   * USART_CR1_IDLEIE   USART_SR_IDLE   Idle Line Detected        (not used)
   * USART_CR1_RXNEIE   USART_SR_RXNE   Rx Data Ready to be Read
   * "              "   USART_SR_ORE    Overrun Error Detected
   * USART_CR1_PEIE     USART_SR_PE     Parity Error
   *
   * USART_CR2_LBDIE    USART_SR_LBD    Break Flag                (not used)
   * USART_CR3_EIE      USART_SR_FE     Framing Error
   * "           "      USART_SR_NE     Noise Error
   * "           "      USART_SR_ORE    Overrun Error Detected
   */

  flags = enter_critical_section();
  ie = priv->ie;
  if (enable)
    {
      /* Receive an interrupt when their is anything in the Rx data register
       * (or an Rx timeout occurs).
       */

#ifndef CONFIG_SUPPRESS_SERIAL_INTS
#ifdef CONFIG_USART_ERRINTS
      ie |= (USART_CR1_RIE | USART_CR1_PCE);
#else
      ie |= USART_CR1_RIE;
#endif
#endif
    }
  else
    {
//      ie &= ~(USART_CR1_RIE | USART_CR1_PCE );
       ie &= ~(USART_CR1_RIE);
    }

  /* Then set the new interrupt state */

  up_restoreusartint(priv, ie);
  leave_critical_section(flags);
}

/****************************************************************************
 * Name: up_rxavailable
 *
 * Description:
 *   Return true if the receive register is not empty
 *
 ****************************************************************************/

//#if defined(SERIAL_HAVE_TXDMA_OPS) || defined(SERIAL_HAVE_NODMA_OPS)
static bool up_rxavailable(struct uart_dev_s *dev)
{
  hc32_print("check %s line %d\n", __func__, __LINE__);

  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  M4_USART_TypeDef *base = (M4_USART_TypeDef *)priv->usartbase;
  return ((up_serialin(priv, base->SR) & USART_SR_RXNE) != 0);
}
//#endif

/****************************************************************************
 * Name: up_rxflowcontrol
 *
 * Description:
 *   Called when Rx buffer is full (or exceeds configured watermark levels
 *   if CONFIG_SERIAL_IFLOWCONTROL_WATERMARKS is defined).
 *   Return true if UART activated RX flow control to block more incoming
 *   data
 *
 * Input Parameters:
 *   dev       - UART device instance
 *   nbuffered - the number of characters currently buffered
 *               (if CONFIG_SERIAL_IFLOWCONTROL_WATERMARKS is
 *               not defined the value will be 0 for an empty buffer or the
 *               defined buffer size for a full buffer)
 *   upper     - true indicates the upper watermark was crossed where
 *               false indicates the lower watermark has been crossed
 *
 * Returned Value:
 *   true if RX flow control activated.
 *
 ****************************************************************************/

#ifdef CONFIG_SERIAL_IFLOWCONTROL
static bool up_rxflowcontrol(struct uart_dev_s *dev,
                             unsigned int nbuffered, bool upper)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  hc32_print("check %s line %d\n", __func__, __LINE__);

#if defined(CONFIG_SERIAL_IFLOWCONTROL_WATERMARKS) && \
    defined(CONFIG_HC32_FLOWCONTROL_BROKEN)
  if (priv->iflow && (priv->rts_gpio != 0))
    {
      /* Assert/de-assert nRTS set it high resume/stop sending */

      hc32_gpiowrite(priv->rts_gpio, upper);

      if (upper)
        {
          /* With heavy Rx traffic, RXNE might be set and data pending.
           * Returning 'true' in such case would cause RXNE left unhandled
           * and causing interrupt storm. Sending end might be also be slow
           * to react on nRTS, and returning 'true' here would prevent
           * processing that data.
           *
           * Therefore, return 'false' so input data is still being processed
           * until sending end reacts on nRTS signal and stops sending more.
           */

          return false;
        }

      return upper;
    }

#else
  if (priv->iflow)
    {
      /* Is the RX buffer full? */

      if (upper)
        {
          /* Disable Rx interrupt to prevent more data being from
           * peripheral.  When hardware RTS is enabled, this will
           * prevent more data from coming in.
           *
           * This function is only called when UART recv buffer is full,
           * that is: "dev->recv.head + 1 == dev->recv.tail".
           *
           * Logic in "uart_read" will automatically toggle Rx interrupts
           * when buffer is read empty and thus we do not have to re-
           * enable Rx interrupts.
           */

          uart_disablerxint(dev);
          return true;
        }

      /* No.. The RX buffer is empty */

      else
        {
          /* We might leave Rx interrupt disabled if full recv buffer was
           * read empty.  Enable Rx interrupt to make sure that more input is
           * received.
           */

          uart_enablerxint(dev);
        }
    }
#endif

  return false;
}
#endif


/****************************************************************************
 * Name: up_send
 *
 * Description:
 *   This method will send one byte on the USART
 *
 ****************************************************************************/

static void up_send(struct uart_dev_s *dev, int ch)
{
    while(Set != USART_GetStatus(USART_UNIT, USART_FLAG_TXE));
    USART_SendData(USART_UNIT, (uint16_t)(ch));
}

/****************************************************************************
 * Name: up_txint
 *
 * Description:
 *   Call to enable or disable TX interrupts
 *
 ****************************************************************************/

static void up_txint(struct uart_dev_s *dev, bool enable)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  irqstate_t flags;

  /* USART transmit interrupts:
   *
   * Enable             Status          Meaning                 Usage
   * ------------------ --------------- ----------------------- ----------
   * USART_CR1_TCIE     USART_SR_TC     Tx Complete             (RS-485)
   * USART_CR1_TXEIE    USART_SR_TXE    Tx Data Register Empty
   * USART_CR3_CTSIE    USART_SR_CTS    CTS flag                (not used)
   */

  flags = enter_critical_section();
  if (enable)
    {
      /* Set to receive an interrupt when the TX data register is empty */

#ifndef CONFIG_SUPPRESS_SERIAL_INTS
      uint16_t ie = priv->ie | USART_CR1_TXEIE;

      /* If RS-485 is supported on this U[S]ART, then also enable the
       * transmission complete interrupt.
       */

#  ifdef HAVE_RS485
      if (priv->rs485_dir_gpio != 0)
        {
          ie |= USART_CR1_TCIE;
        }
#  endif

#  ifdef CONFIG_HC32_SERIALBRK_BSDCOMPAT
      if (priv->ie & USART_CR1_IE_BREAK_INPROGRESS)
        {
          leave_critical_section(flags);
          return;
        }
#  endif

      up_restoreusartint(priv, ie);

#else
      /* Fake a TX interrupt here by just calling uart_xmitchars() with
       * interrupts disabled (note this may recurse).
       */

      uart_xmitchars(dev);
#endif
    }
  else
    {
      /* Disable the TX interrupt */

      up_restoreusartint(priv, priv->ie & ~USART_CR1_TXEIE);
    }

  leave_critical_section(flags);
}

/****************************************************************************
 * Name: up_txready
 *
 * Description:
 *   Return true if the transmit data register is empty
 *
 ****************************************************************************/

static bool up_txready(struct uart_dev_s *dev)
{
  struct up_dev_s *priv = (struct up_dev_s *)dev->priv;
  M4_USART_TypeDef *base = (M4_USART_TypeDef *)priv->usartbase;
  return ((up_serialin(priv, base->SR) & USART_SR_TXE) != 0);
}

/****************************************************************************
 * Name: up_pm_notify
 *
 * Description:
 *   Notify the driver of new power state. This callback is  called after
 *   all drivers have had the opportunity to prepare for the new power state.
 *
 * Input Parameters:
 *
 *    cb - Returned to the driver. The driver version of the callback
 *         structure may include additional, driver-specific state data at
 *         the end of the structure.
 *
 *    pmstate - Identifies the new PM state
 *
 * Returned Value:
 *   None - The driver already agreed to transition to the low power
 *   consumption state when when it returned OK to the prepare() call.
 *
 *
 ****************************************************************************/

#ifdef CONFIG_PM
static void up_pm_notify(struct pm_callback_s *cb, int domain,
                         enum pm_state_e pmstate)
{
  switch (pmstate)
    {
      case(PM_NORMAL):
        {
          /* Logic for PM_NORMAL goes here */
        }
        break;

      case(PM_IDLE):
        {
          /* Logic for PM_IDLE goes here */
        }
        break;

      case(PM_STANDBY):
        {
          /* Logic for PM_STANDBY goes here */
        }
        break;

      case(PM_SLEEP):
        {
          /* Logic for PM_SLEEP goes here */
        }
        break;

      default:
        {
          /* Should not get here */
        }
        break;
    }
}
#endif

/****************************************************************************
 * Name: up_pm_prepare
 *
 * Description:
 *   Request the driver to prepare for a new power state. This is a warning
 *   that the system is about to enter into a new power state. The driver
 *   should begin whatever operations that may be required to enter power
 *   state. The driver may abort the state change mode by returning a
 *   non-zero value from the callback function.
 *
 * Input Parameters:
 *
 *    cb - Returned to the driver. The driver version of the callback
 *         structure may include additional, driver-specific state data at
 *         the end of the structure.
 *
 *    pmstate - Identifies the new PM state
 *
 * Returned Value:
 *   Zero - (OK) means the event was successfully processed and that the
 *          driver is prepared for the PM state change.
 *
 *   Non-zero - means that the driver is not prepared to perform the tasks
 *              needed achieve this power setting and will cause the state
 *              change to be aborted. NOTE: The prepare() method will also
 *              be called when reverting from lower back to higher power
 *              consumption modes (say because another driver refused a
 *              lower power state change). Drivers are not permitted to
 *              return non-zero values when reverting back to higher power
 *              consumption modes!
 *
 *
 ****************************************************************************/

#ifdef CONFIG_PM
static int up_pm_prepare(struct pm_callback_s *cb, int domain,
                         enum pm_state_e pmstate)
{
  /* Logic to prepare for a reduced power state goes here. */

  return OK;
}
#endif
#endif /* HAVE_SERIALDRIVER */
#endif /* USE_SERIALDRIVER */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef USE_SERIALDRIVER

/****************************************************************************
 * Name: hc32_serial_get_uart
 *
 * Description:
 *   Get serial driver structure for HC32 USART
 *
 ****************************************************************************/

#ifdef HAVE_SERIALDRIVER
FAR uart_dev_t *hc32_serial_get_uart(int uart_num)
{
  int uart_idx = uart_num - 1;

  if (uart_idx < 0 || uart_idx >= HC32_NUSART || !g_uart_devs[uart_idx])
    {
      return NULL;
    }

  if (!g_uart_devs[uart_idx]->initialized)
    {
      return NULL;
    }

  return &g_uart_devs[uart_idx]->dev;
}
#endif /* HAVE_SERIALDRIVER */

/****************************************************************************
 * Name: arm_earlyserialinit
 *
 * Description:
 *   Performs the low level USART initialization early in debug so that the
 *   serial console will be available during bootup.  This must be called
 *   before arm_serialinit.
 *
 ****************************************************************************/

#ifdef USE_EARLYSERIALINIT
void arm_earlyserialinit(void)
{

#ifdef HAVE_SERIALDRIVER
  unsigned i;

  /* Disable all USART interrupts */

  for (i = 0; i < HC32_NUSART; i++)
    {
      if (g_uart_devs[i])
        {
          up_disableusartint(g_uart_devs[i], NULL);
        }
    }

  /* Configure whichever one is the console */

#if CONSOLE_UART > 0
  up_setup(&g_uart_devs[CONSOLE_UART - 1]->dev);
#endif
#endif /* HAVE UART */
}
#endif

/****************************************************************************
 * Name: arm_serialinit
 *
 * Description:
 *   Register serial console and serial ports.  This assumes
 *   that arm_earlyserialinit was called previously.
 *
 ****************************************************************************/

void arm_serialinit(void)
{
#ifdef HAVE_SERIALDRIVER
  char devname[16];
  unsigned i;
  unsigned minor = 0;
#ifdef CONFIG_PM
  int ret;
#endif

  /* Register to receive power management callbacks */

#ifdef CONFIG_PM
  ret = pm_register(&g_serialcb);
  DEBUGASSERT(ret == OK);
  UNUSED(ret);
#endif

  /* Register the console */

#if CONSOLE_UART > 0
  uart_register("/dev/console", &g_uart_devs[CONSOLE_UART - 1]->dev);

#ifndef CONFIG_HC32_SERIAL_DISABLE_REORDERING
  /* If not disabled, register the console UART to ttyS0 and exclude
   * it from initializing it further down
   */

  uart_register("/dev/ttyS0", &g_uart_devs[CONSOLE_UART - 1]->dev);
  minor = 1;
#endif

#endif /* CONSOLE_UART > 0 */

  /* Register all remaining USARTs */

  strcpy(devname, "/dev/ttySx");

  for (i = 0; i < HC32_NUSART; i++)
    {
      /* Don't create a device for non-configured ports. */

      if (g_uart_devs[i] == 0)
        {
          continue;
        }

#ifndef CONFIG_HC32_SERIAL_DISABLE_REORDERING
      /* Don't create a device for the console - we did that above */

      if (g_uart_devs[i]->dev.isconsole)
        {
          continue;
        }
#endif

      /* Register USARTs as devices in increasing order */

      devname[9] = '0' + minor++;
      uart_register(devname, &g_uart_devs[i]->dev);
    }
#endif /* HAVE UART */
}


/****************************************************************************
 * Name: up_putc
 *
 * Description:
 *   Provide priority, low-level access to support OS debug writes
 *
 ****************************************************************************/

int up_putc(int ch)
{
#if CONSOLE_UART > 0
  struct up_dev_s *priv = g_uart_devs[CONSOLE_UART - 1];
  uint16_t ie;

  up_disableusartint(priv, &ie);

  /* Check for LF */

  if (ch == '\n')
    {
      /* Add CR */

      arm_lowputc('\r');
    }

  arm_lowputc(ch);
  up_restoreusartint(priv, ie);
#endif
  return ch;
}

#else /* USE_SERIALDRIVER */

/****************************************************************************
 * Name: up_putc
 *
 * Description:
 *   Provide priority, low-level access to support OS debug writes
 *
 ****************************************************************************/

int up_putc(int ch)
{
#if CONSOLE_UART > 0
  /* Check for LF */

  if (ch == '\n')
    {
      /* Add CR */

      arm_lowputc('\r');
    }

  arm_lowputc(ch);
#endif
  return ch;
}

#endif /* USE_SERIALDRIVER */
