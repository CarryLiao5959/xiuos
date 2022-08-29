include $(KERNEL_ROOT)/.config

ifeq ($(CONFIG_ADD_NUTTX_FETURES),y)
    include $(APPDIR)/Make.defs
    CSRCS += g8s.c
    include $(APPDIR)/Application.mk
endif

ifeq ($(CONFIG_ADD_XIZI_FETURES),y)
    SRC_FILES := g8s.c
    include $(KERNEL_ROOT)/compiler.mk
endif

