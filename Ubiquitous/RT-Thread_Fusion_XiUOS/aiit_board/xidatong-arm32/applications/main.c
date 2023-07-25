/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-04-29     tyustli      first version
 *
 */

#include <rtdevice.h>
#include <rtthread.h>
#include "drv_gpio.h"
#include <board.h>
#include "fsl_gpio.h"
#include "board/hardware/ch438/ch438.h"
/* defined the LED pin: GPIO1_IO9 */
#define LED0_PIN               GET_PIN(1,9)
#define GPIO_AD_B0_01_PIN      GET_PIN(2,1)
extern int FrameworkInit(void);
int main(void)
{
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

    /* set GPIO_AD_B0_01_PIN pin mode to output */
    rt_pin_mode(GPIO_AD_B0_01_PIN, PIN_MODE_OUTPUT);
    /* write high level to GPIO_AD_B0_01_PIN */
    //rt_pin_write(GPIO_AD_B0_01_PIN, PIN_HIGH);
		rt_pin_write(GPIO_AD_B0_01_PIN, PIN_LOW);

    int pin_state;
    /* read and print GPIO_AD_B0_01_PIN level */
    pin_state = rt_pin_read(GPIO_AD_B0_01_PIN);
    rt_kprintf("GPIO_AD_B0_01_PIN level: %d\n", pin_state);

    rt_kprintf("XIUOS xidatong build %s %s\n", __DATE__, __TIME__);
    FrameworkInit();
    while (1)
    {
        rt_thread_mdelay(5000);
    }
}

#ifdef BSP_USING_SDRAM
static void sdram_test2(void)
{
    char *p =NULL;
	p = rt_malloc(1024*1024*8);
	if(p == NULL)
	{
		rt_kprintf("apply for 8MB memory fail ~!!!");
	}
	else
	{
		rt_kprintf("appyle for 8MB memory success!!!");
	}
	rt_free(p);
}
MSH_CMD_EXPORT(sdram_test2, sdram test2);
#endif

#ifdef BSP_USB1_HOST
#include <dfs_posix.h>
static char test_data[120], buffer[120];
#define TEST_FN     "/testusb.c"
void readwrite(const char* filename)
{
    int fd;
    int index, length;

    fd = open(TEST_FN, O_WRONLY | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        rt_kprintf("open file for write failed\n");
        return;
    }

    for (index = 0; index < sizeof(test_data); index ++)
    {
        test_data[index] = index + 27;
    }

    length = write(fd, test_data, sizeof(test_data));
    if (length != sizeof(test_data))
    {
        rt_kprintf("write data failed\n");
        close(fd);
        return;
    }

    close(fd);

    fd = open(TEST_FN, O_RDONLY, 0);
    if (fd < 0)
    {
        rt_kprintf("check: open file for read failed\n");
        return;
    }

    length = read(fd, buffer, sizeof(buffer));
    if (length != sizeof(buffer))
    {
        rt_kprintf("check: read file failed\n");
        close(fd);
        return;
    }

    for (index = 0; index < sizeof(test_data); index ++)
    {
        if (test_data[index] != buffer[index])
        {
            rt_kprintf("check: check data failed at %d\n", index);
            close(fd);
            return;
        }
    }

    rt_kprintf("usb host read/write udisk successful\r\n");

    close(fd);
}

MSH_CMD_EXPORT(readwrite, usb host read write test);
#endif

#ifdef BSP_USING_CAN
static rt_device_t can_dev;            /* CAN 设备句柄 */
static void can2_test(void)
{
   
    struct rt_can_msg msg = {0};
    rt_err_t res;
    rt_size_t  size;
    rt_thread_t thread;
    /* 查找 CAN 设备 */
    can_dev = rt_device_find("can2");
    if (!can_dev)
    {
        rt_kprintf("find can2 failed!\n");
        return;
    }

    /* 以中断接收及发送方式打开 CAN 设备 */
    res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    RT_ASSERT(res == RT_EOK);
    /* 创建数据接收线程 */
    msg.id = 0x78;              /* ID 为 0x78 */
    msg.ide = RT_CAN_STDID;     /* 标准格式 */
    msg.rtr = RT_CAN_DTR;       /* 数据帧 */
    msg.len = 8;                /* 数据长度为 8 */
    /* 待发送的 8 字节数据 */
    msg.data[0] = 0x00;
    msg.data[1] = 0x11;
    msg.data[2] = 0x22;
    msg.data[3] = 0x33;
    msg.data[4] = 0x44;
    msg.data[5] = 0x55;
    msg.data[6] = 0x66;
    msg.data[7] = 0x77;
    /* 发送一帧 CAN 数据 */
    size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
    if (size == 0)
    {
        rt_kprintf("can dev write data failed!\n");
    }
    rt_device_close(can_dev);
   
}
MSH_CMD_EXPORT(can2_test, can2 test);
#endif

#ifdef BSP_USING_I2C1
#include <touch.h>
#include "drv_touch.h"
static rt_thread_t  ok_1052_touch_thread = RT_NULL;
static rt_sem_t     ok_1052_touch_sem = RT_NULL;
static rt_device_t  dev = RT_NULL;

/* 读取数据线程入口函数 */
static void ok_1052_touch_entry(void *parameter)
{
    TouchDataStandard_drv *read_data;
    read_data = (TouchDataStandard_drv *)rt_malloc(sizeof(TouchDataStandard_drv) * 1);

    while (1)
    {
        /* 请求信号量 */
        rt_sem_take(ok_1052_touch_sem, RT_WAITING_FOREVER);
        /* 读取五个点的触摸信息 */
        if (rt_device_read(dev, 0, read_data, 1) == 1)
        {
           
                
                    rt_kprintf("X:%d Y:%d \n",
                                read_data->x,
                                read_data->y);
            
        }
        /* 打开中断 */
        rt_device_control(dev, RT_TOUCH_CTRL_ENABLE_INT, RT_NULL);
    }
}

/* 接收回调函数 */
static rt_err_t rx_callback(rt_device_t dev, rt_size_t size)
{
    /* 关闭中断 */
    rt_device_control(dev, RT_TOUCH_CTRL_DISABLE_INT, RT_NULL);
    /* 释放信号量 */
    rt_sem_release(ok_1052_touch_sem);
    return 0;
}

static int ok_1052_touch_sample(void)
{
    /* 查找 Touch 设备 */
    dev = rt_device_find("touch");

    if (dev == RT_NULL)
    {
        rt_kprintf("can't find device:%s\n", "touch");
        return -1;
    }
    /* 以中断的方式打开设备 */
    if (rt_device_open(dev, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("open device failed!");
        return -1;
    }
    /* 设置接收回调 */
    rt_device_set_rx_indicate(dev, rx_callback);
    /* 创建信号量 */
    ok_1052_touch_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_PRIO);

    if (ok_1052_touch_sem == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }
    /* 创建读取数据线程 */
    ok_1052_touch_thread = rt_thread_create("touch1052",
                                     ok_1052_touch_entry,
                                     RT_NULL,
                                     512,
                                     5,
                                     10);
    /* 启动线程 */
    if (ok_1052_touch_thread != RT_NULL)
        rt_thread_startup(ok_1052_touch_thread);

    return 0;
}
MSH_CMD_EXPORT(ok_1052_touch_sample, ok_1052_touch_sample);
#endif
