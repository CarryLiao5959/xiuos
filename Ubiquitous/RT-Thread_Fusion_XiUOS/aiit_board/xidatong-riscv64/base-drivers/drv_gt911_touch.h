/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-13     RiceChen     the first version
 */

#ifndef __DRV_GT911_TOUCH_H__
#define __DRV_GT911_TOUCH_H__

#include <touch.h>

#define GT911_ADDR_LEN          2
#define GT911_REGITER_LEN       2
#define GT911_MAX_TOUCH         5
#define GT911_POINT_INFO_NUM   5

#define GT911_ADDRESS_HIGH      0x5D
#define GT911_ADDRESS_LOW       0x14

#define GT911_COMMAND_REG       0x8040  //GT911控制寄存器
#define GT911_CONFIG_REG        0x8047  //GT911配置起始地址寄存器

#define GT911_PRODUCT_ID        0x8140  //GT911产品ID寄存器
#define GT911_VENDOR_ID         0x814A  //GT911模组选项信息
#define GT911_READ_STATUS       0x814E  //GT911当前检测到的触摸情况

#define GT911_POINT1_REG        0x814F  //第一个触摸点数据地址
#define GT911_POINT2_REG        0x8157  //第二个触摸点数据地址
#define GT911_POINT3_REG        0x815F  //第三个触摸点数据地址
#define GT911_POINT4_REG        0x8167  //第四个触摸点数据地址
#define GT911_POINT5_REG        0x816F  //第五个触摸点数据地址

#define GT911_CHECK_SUM         0x80FF  //GT911校验和寄存器

int rt_hw_gt911_init(const char *name, struct rt_touch_config *cfg);
int rt_hw_gt911_port(void);

#endif
