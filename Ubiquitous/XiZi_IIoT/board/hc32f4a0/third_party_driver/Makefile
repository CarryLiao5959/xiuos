SRC_DIR += common

ifeq ($(CONFIG_BSP_USING_UART),y)
  SRC_DIR += usart
endif

ifeq ($(CONFIG_BSP_USING_GPIO),y)
  SRC_DIR += gpio
endif

ifeq ($(CONFIG_BSP_USING_LWIP),y)
  SRC_DIR += ethernet
endif

ifeq ($(CONFIG_BSP_USING_SPI),y)
  SRC_DIR += spi
endif

ifeq ($(CONFIG_BSP_USING_I2C),y)
  SRC_DIR += i2c
endif

ifeq ($(CONFIG_BSP_USING_SDIO),y)
  SRC_DIR += sdio
endif

ifeq ($(CONFIG_BSP_USING_USB),y)
  SRC_DIR += usb
endif

include $(KERNEL_ROOT)/compiler.mk
