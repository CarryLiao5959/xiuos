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
* @file drv_soft_i2c.h
* @brief define xidatong-riscv64-board i2c function and struct
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2022-09-30
*/

#ifndef __DRV_SOFT_I2C_H__
#define __DRV_SOFT_I2C_H__


#include <i2c.h>
#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rt_i2c_config
{
    char *name;
    rt_uint8_t scl;
    rt_uint8_t sda;
};

/* rt i2c dirver class */
struct rt_i2c
{
    struct rt_i2c_bit_ops ops;
    struct rt_i2c_bus_device i2c2_bus;
};

#ifdef BSP_USING_I2C0
#define I2C0_BUS_CONFIG                                  \
    {                                                    \
        .scl = BSP_I2C0_SCL_PIN,                         \
        .sda = BSP_I2C0_SDA_PIN,                         \
        .name = "i2c0",                              \
    }
#endif

#ifdef BSP_USING_I2C1
#define I2C1_BUS_CONFIG                                   \
    {                                                    \
        .scl = BSP_I2C1_SCL_PIN,                         \
        .sda = BSP_I2C1_SDA_PIN,                         \
        .name = "i2c1",                              \
    }
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DRV_SOFT_I2C_H__ */
