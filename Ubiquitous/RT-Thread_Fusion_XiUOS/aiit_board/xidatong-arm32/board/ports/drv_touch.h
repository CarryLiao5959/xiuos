/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
* @file dev_touch.h
* @brief define touch dev function using bus driver framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-24
*/

#ifndef DEV_TOUCH_H
#define DEV_TOUCH_H

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif


#define CFG_GROUP_LEN(p_cfg_grp)  (sizeof(p_cfg_grp) / sizeof(p_cfg_grp[0]))
#define OK1052_TOUCH_NAME         "touch"
#define OK1052_TOUCH_IIC_NAME     "i2c1"  
#define BSP_OK1052_IRQ_PIN         (62)
/*设定使用的电容屏IIC设备地址*/
#define GTP_ADDRESS            0xBA
#define RT_GTP_ADDRESS         (GTP_ADDRESS >> 1)
#define GTP_MAX_HEIGHT   272
#define GTP_MAX_WIDTH    480
#define GTP_INT_TRIGGER  0
#define GTP_MAX_TOUCH    5

#define GTP_CONFIG_MAX_LENGTH 240
#define GTP_ADDR_LENGTH       2

// Registers define
#define GTP_READ_COOR_ADDR    0x814E
#define GTP_REG_SLEEP         0x8040
#define GTP_REG_SENSOR_ID     0x814A
#define GTP_REG_CONFIG_DATA   0x8047
#define GTP_REG_VERSION       0x8140

typedef struct
{
    uint16_t x;
    uint16_t y;
}TouchDataStandard_drv;




struct rt_ok1052
{
    struct rt_touch_device parent;
    struct rt_i2c_bus_device  *i2c;
};
typedef struct rt_ok1052 *rt_ok1052_t;



#ifdef __cplusplus
}
#endif

#endif
