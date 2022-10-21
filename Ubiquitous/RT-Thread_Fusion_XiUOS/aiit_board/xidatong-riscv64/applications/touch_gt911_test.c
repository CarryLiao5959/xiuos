/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-13     RiceChen     the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_gt911_touch.h>
#include <touch.h>

static rt_thread_t  gt911_thread = RT_NULL;
static rt_device_t touch_dev = RT_NULL;
static struct rt_touch_data *read_data;
static struct rt_touch_info info;

static void gt911_entry(void *parameter)
{
    read_data = (struct rt_touch_data *)rt_malloc(sizeof(struct rt_touch_data) * info.point_num);

    while (1)
    {   
        rt_uint8_t read_num = rt_device_read(touch_dev, 0, read_data, 5);
        for (rt_uint8_t i = 0; i < read_num; i++) 
        {
            if (read_data[i].event == RT_TOUCH_EVENT_DOWN || read_data[i].event == RT_TOUCH_EVENT_MOVE) 
                {
                    rt_kprintf("track id: %d\n", read_data[i].track_id);
                    rt_kprintf("event: %s\n", read_data[i].event == RT_TOUCH_EVENT_DOWN ? "RT_TOUCH_EVENT_DOWN" : "RT_TOUCH_EVENT_UP");
                    rt_kprintf("x: %d, y: %d\n", read_data[i].x_coordinate, read_data[i].y_coordinate);
                    rt_kprintf("timestamp: %d\n", read_data[i].timestamp);
                    rt_kprintf("\n");
                }
        }
        
        //rt_thread_mdelay(20);
    }
}

/* Test function */
int gt911_sample()
{
    /* Accessing the touch device */
    touch_dev = rt_device_find("touch");
    if (touch_dev == RT_NULL)
    {
        rt_kprintf("can't find device:%s\n","touch");
        return -1;
    }

    if (rt_device_open(touch_dev, RT_DEVICE_FLAG_RDONLY) != RT_EOK)
    {
        rt_kprintf("open device failed!");
        return -1;
    }

    /* read ID */
    rt_uint8_t read_id[5];
    rt_device_control(touch_dev, RT_TOUCH_CTRL_GET_ID, read_id);
    rt_kprintf("id = GT%d%d%d \n", read_id[0] - '0', read_id[1] - '0', read_id[2] - '0');

    /* Get device information */
    rt_device_control(touch_dev, RT_TOUCH_CTRL_GET_INFO, &info);
    rt_kprintf("type       :%d \n", info.type);                       /* 类型：电容型/电阻型*/
    rt_kprintf("vendor     :%d \n", info.vendor);                     /* 厂商 */
    rt_kprintf("point_num  :%d \n", info.point_num);                  /* 支持的触点个数 */
    rt_kprintf("range_x    :%d \n", info.range_x);                    /* X 轴分辨率 */
    rt_kprintf("range_y    :%d \n", info.range_y);                    /* Y 轴分辨率*/

    gt911_thread = rt_thread_create("gt911",
                                     gt911_entry,
                                     RT_NULL,
                                     2048,
                                     18,
                                     5);

    if (gt911_thread != RT_NULL)
        rt_thread_startup(gt911_thread);

    return 0;
}
MSH_CMD_EXPORT(gt911_sample, a gt911 sample);
