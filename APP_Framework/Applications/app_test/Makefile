SRC_FILES := test_shell.c

ifeq ($(CONFIG_USER_TEST_ADC),y)
	SRC_FILES += test_adc.c
endif

ifeq ($(CONFIG_USER_TEST_DAC),y)
	SRC_FILES += test_dac.c
endif

ifeq ($(CONFIG_USER_TEST_SEMC),y)
	SRC_FILES += test_extsram.c
endif

ifeq ($(CONFIG_ADD_XIZI_FETURES),y)
include $(KERNEL_ROOT)/compiler.mk
endif

include $(KERNEL_ROOT)/.config
ifeq ($(CONFIG_ADD_NUTTX_FETURES),y)
    include $(APPDIR)/Make.defs
    CSRCS += test_extsram.c
    include $(APPDIR)/Application.mk
endif

