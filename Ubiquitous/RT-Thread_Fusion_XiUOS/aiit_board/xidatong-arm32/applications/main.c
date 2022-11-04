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
extern int FrameworkInit(void);
int main(void)
{   
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_kprintf("XIUOS xidatong build %s %s\n",__DATE__,__TIME__);
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