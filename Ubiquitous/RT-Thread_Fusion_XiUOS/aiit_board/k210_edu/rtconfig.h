#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* XIUOS Rt-thread Configuration */

#define ROOT_DIR "../../../.."
#define BSP_DIR "."
#define RT_Thread_DIR "../.."
#define RTT_DIR "../../rt-thread"
#define BOARD_K210_EVB

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_USING_SMP
#define RT_CPUS_NR 2
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 4096
#define SYSTEM_THREAD_STACK_SIZE 4096
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 2048

/* kservice optimization */

#define RT_DEBUG
#define RT_DEBUG_COLOR
#define RT_DEBUG_INIT_CONFIG
#define RT_DEBUG_INIT 1

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
#define RT_USING_SIGNALS

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_SLAB
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uarths"
#define RT_VER_NUM 0x40004
#define ARCH_CPU_64BIT
#define ARCH_RISCV
#define ARCH_RISCV_FPU
#define ARCH_RISCV_FPU_S
#define ARCH_RISCV64

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 10240
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */

#define RT_USING_CPLUSPLUS

/* Command shell */

#define RT_USING_FINSH
#define RT_USING_MSH
#define FINSH_USING_MSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 20480
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_CMD_SIZE 80
#define MSH_USING_BUILT_IN_COMMANDS
#define FINSH_USING_DESCRIPTION
#define FINSH_ARG_MAX 10

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 16
#define DFS_FILESYSTEM_TYPES_MAX 16
#define DFS_FD_MAX 64
#define RT_USING_DFS_ELMFAT

/* elm-chan's FatFs, Generic FAT Filesystem Module */

#define RT_DFS_ELM_CODE_PAGE 437
#define RT_DFS_ELM_WORD_ACCESS
#define RT_DFS_ELM_USE_LFN_3
#define RT_DFS_ELM_USE_LFN 3
#define RT_DFS_ELM_LFN_UNICODE_0
#define RT_DFS_ELM_LFN_UNICODE 0
#define RT_DFS_ELM_MAX_LFN 255
#define RT_DFS_ELM_DRIVES 2
#define RT_DFS_ELM_MAX_SECTOR_SIZE 4096
#define RT_DFS_ELM_REENTRANT
#define RT_DFS_ELM_MUTEX_TIMEOUT 3000
#define RT_USING_DFS_DEVFS

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SYSTEM_WORKQUEUE
#define RT_SYSTEM_WORKQUEUE_STACKSIZE 4096
#define RT_SYSTEM_WORKQUEUE_PRIORITY 23
#define RT_USING_SERIAL
#define RT_USING_SERIAL_V1
#define RT_SERIAL_USING_DMA
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_PIN
#define RT_USING_SPI
#define RT_USING_SPI_MSD

/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_PTHREADS
#define PTHREAD_NUM_MAX 8
#define RT_USING_POSIX
#define RT_LIBC_USING_TIME
#define RT_LIBC_DEFAULT_TIMEZONE 8

/* Network */

/* Socket abstraction layer */

#define RT_USING_SAL
#define SAL_INTERNET_CHECK
#define SAL_SOCKETS_NUM 16

/* Network interface device */

#define RT_USING_NETDEV
#define NETDEV_USING_IFCONFIG
#define NETDEV_USING_PING
#define NETDEV_USING_NETSTAT
#define NETDEV_USING_AUTO_DEFAULT
#define NETDEV_IPV4 1
#define NETDEV_IPV6 0

/* light weight TCP/IP stack */


/* AT commands */


/* VBUS(Virtual Software BUS) */


/* Utilities */


/* RT-Thread Utestcases */


/* Board Drivers Config */

/* On-chip Peripheral Drivers */

#define __STACKSIZE__ 4096
#define BSP_USING_UART_HS
#define BSP_USING_SOFT_SPI
#define BSP_SOFT_SPI_CLK_PIN 26
#define BSP_SOFT_SPI_MOSI_PIN 27
#define BSP_SOFT_SPI_MISO_PIN 25
#define RT_USING_SOFT_SPI_CS0
#define RT_USING_SOFT_SPI_CS0_PIN 28

/* Onboard Peripheral Drivers */

#define BSP_USING_LCD
#define BSP_LCD_CS_PIN 37
#define BSP_LCD_WR_PIN 38
#define BSP_LCD_DC_PIN 39
#define BSP_LCD_RST_PIN 36
#define BSP_LCD_BACKLIGHT_PIN -1
#define BSP_LCD_BACKLIGHT_ACTIVE_LOW
#define BSP_LCD_CLK_FREQ 15000000
#define BSP_BOARD_KD233
#define BSP_LCD_X_MAX 320
#define BSP_LCD_Y_MAX 320
#define BSP_USING_DVP

/* The default pin assignment is based on the Maix Duino K210 development board */

#define BSP_DVP_SCCB_SDA_PIN 47
#define BSP_DVP_SCCB_SCLK_PIN 46
#define BSP_DVP_CMOS_RST_PIN 40
#define BSP_DVP_CMOS_VSYNC_PIN 45
#define BSP_DVP_CMOS_PWDN_PIN 41
#define BSP_DVP_CMOS_XCLK_PIN 42
#define BSP_DVP_CMOS_PCLK_PIN 43
#define BSP_DVP_CMOS_HREF_PIN 44
#define BSP_USING_SDCARD

/* Kendryte SDK Config */

#define PKG_KENDRYTE_SDK_VERNUM 0x0055

/* MicroPython */


/* More Drivers */

#define DRV_USING_OV2640
#define OV2640_JPEG_MODE
#define OV2640_X_RESOLUTION_IMAGE_OUTSIZE 240
#define OV2640_Y_RESOLUTION_IMAGE_OUTSIZE 240
#define OV2640_X_IMAGE_WINDOWS_SIZE 400

/* the value must be greater than OV2640_X_RESOLUTION_IMAGE_OUTSIZE */

#define OV2640_Y_IMAGE_WINDOWS_SIZE 400

/* the value must be greater than OV2640_Y_RESOLUTION_IMAGE_OUTSIZE */


/* APP_Framework */

/* Framework */

#define TRANSFORM_LAYER_ATTRIUBUTE
#define ADD_RTTHREAD_FETURES
#define SUPPORT_KNOWING_FRAMEWORK
#define USING_KPU_PROCESSING
#define USING_YOLOV2
#define USING_YOLOV2_JSONPARSER
#define USING_K210_YOLOV2_DETECT

/* Security */


/* Applications */

/* config stack size and priority of main task */

#define MAIN_KTASK_STACK_SIZE 1024

/* ota app  */


/* test app */


/* connection app */


/* control app */

/* knowing app */

#define APPLICATION_KNOWING
#define K210_DETECT_ENTRY

/* sensor app */

#define APPLICATION_SENSOR

/* lib */

#define APP_SELECT_NEWLIB
#define LIB_USING_CJSON

/* LVGL configuration */


#endif
