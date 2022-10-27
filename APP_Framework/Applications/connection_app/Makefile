
ifeq ($(CONFIG_SOCKET_DEMO),y)

include $(KERNEL_ROOT)/.config

ifeq ($(CONFIG_ADD_NUTTX_FETURES),y)
    include $(APPDIR)/Make.defs
    include $(APPDIR)/Application.mk
endif

ifeq ($(CONFIG_ADD_XIZI_FETURES),y)

	ifeq ($(CONFIG_RESOURCES_LWIP),y)
		SRC_DIR += socket_demo
	endif

    include $(KERNEL_ROOT)/compiler.mk
endif

endif
