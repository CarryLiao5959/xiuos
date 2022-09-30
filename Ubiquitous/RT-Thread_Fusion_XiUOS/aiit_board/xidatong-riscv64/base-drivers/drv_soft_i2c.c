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
* @file drv_soft_i2c.c
* @brief support xidatong-riscv64-board i2c function and register to bus framework
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2022-09-30
*/


#include <rtthread.h>
#include <rtdevice.h>
#include <rtdbg.h>
#include "board.h"
#include <gpio_common.h>
#include <gpio.h>
#include <gpiohs.h>
#include <fpioa.h>
#include "drv_soft_i2c.h"
#include <sleep.h>
#include <sysctl.h>


static struct rt_i2c_config i2c_config[] =
{
#ifdef BSP_USING_I2C0
     I2C0_BUS_CONFIG,
#endif
#ifdef BSP_USING_I2C1
     I2C1_BUS_CONFIG
#endif
};

static struct rt_i2c i2c_bus[sizeof(i2c_config) / sizeof(i2c_config[0])] = {0};

/**
 * This function initializes the i2c pin.
 *
 * @param xidatong i2c dirver class.
 */
static void i2c_gpio_init(struct rt_i2c *i2c)
{
    struct rt_i2c_config *cfg = (struct rt_i2c_config *)i2c->ops.data;

    rt_pin_mode(cfg->scl, PIN_MODE_OUTPUT);
    rt_pin_mode(cfg->sda, PIN_MODE_OUTPUT);

    rt_pin_write(cfg->scl, PIN_HIGH);
    rt_pin_write(cfg->sda, PIN_HIGH);
}

/**
 * This function sets the sda pin.
 *
 * @param i2c config class.
 * @param The sda pin state.
 */
static void set_sda(void *data, rt_int32_t state)
{
    struct rt_i2c_config *cfg = (struct rt_i2c_config *)data;
    rt_pin_mode(cfg->sda, PIN_MODE_OUTPUT);    // PIN_MODE_OUTPUT_OD -> PIN_MODE_OUTPUT
    if (state)
    {
        rt_pin_write(cfg->sda, PIN_HIGH);
    }
    else
    {
        rt_pin_write(cfg->sda, PIN_LOW);
    }
}

/**
 * This function sets the scl pin.
 *
 * @param i2c config class.
 * @param The scl pin state.
 */
static void set_scl(void *data, rt_int32_t state)
{
    struct rt_i2c_config *cfg = (struct rt_i2c_config *)data;
    rt_pin_mode(cfg->scl, PIN_MODE_OUTPUT);   // PIN_MODE_OUTPUT_OD -> PIN_MODE_OUTPUT
    if (state)
    {
        rt_pin_write(cfg->scl, PIN_HIGH);
    }
    else
    {
        rt_pin_write(cfg->scl, PIN_LOW);
    }
}

/**
 * This function gets the sda pin state.
 *
 * @param The sda pin state.
 */
static rt_int32_t get_sda(void *data)
{
    struct rt_i2c_config *cfg = (struct rt_i2c_config *)data;
    rt_pin_mode(cfg->sda, PIN_MODE_INPUT_PULLUP);
    return rt_pin_read(cfg->sda);
}

/**
 * This function gets the scl pin state.
 *
 * @param The scl pin state.
 */
static rt_int32_t get_scl(void *data)
{
    struct rt_i2c_config *cfg = (struct rt_i2c_config *)data;
    rt_pin_mode(cfg->scl, PIN_MODE_INPUT_PULLUP);
    return rt_pin_read(cfg->scl);
}

static const struct rt_i2c_bit_ops bit_ops =
    {
        .data = RT_NULL,
        .set_sda = set_sda,
        .set_scl = set_scl,
        .get_sda = get_sda,
        .get_scl = get_scl,
        .udelay = usleep,
        .delay_us = 1,
        .timeout = 100};

/**
 * if i2c is locked, this function will unlock it
 *
 * @param xidatong config class
 *
 * @return RT_EOK indicates successful unlock.
 */
static rt_err_t I2cBusReset(const struct rt_i2c_config *cfg)
{
    rt_int32_t i = 0; 
    if (PIN_LOW == rt_pin_read(cfg->sda))
    {
        while (i++ < 9)
        {
            rt_pin_write(cfg->scl, PIN_HIGH);
            usleep(100);
            rt_pin_write(cfg->scl, PIN_LOW);
            usleep(100);
        }
    }
    if (PIN_LOW == rt_pin_read(cfg->sda))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

/* I2C initialization function */
int rt_hw_i2c_init(void)
{
    rt_err_t result;

    for (int i = 0; i < sizeof(i2c_bus) / sizeof(struct rt_i2c); i++)
    {
        i2c_bus[i].ops = bit_ops;
        i2c_bus[i].ops.data = (void *)&i2c_config[i];
        i2c_bus[i].i2c2_bus.priv = &i2c_bus[i].ops;
        i2c_gpio_init(&i2c_bus[i]);
        result = rt_i2c_bit_add_bus(&i2c_bus[i].i2c2_bus, i2c_config[i].name);
        RT_ASSERT(result == RT_EOK);
        I2cBusReset(&i2c_config[i]);

        LOG_D("software simulation %s init done, pin scl: %d, pin sda %d",
              i2c_config[i].name,
              i2c_config[i].scl,
              i2c_config[i].sda);
    }

    return RT_EOK;
}
INIT_BOARD_EXPORT(rt_hw_i2c_init);
