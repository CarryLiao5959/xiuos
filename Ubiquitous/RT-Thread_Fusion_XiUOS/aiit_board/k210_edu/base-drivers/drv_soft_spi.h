/*
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-19     xiaoyu     first version
 */

#ifndef __DRV_SOFT_SPI_H_
#define __DRV_SOFT_SPI_H_
#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>
#include <drivers/pin.h>
/* k210 soft spi config */
#define SOFT_SPI_BUS_CONFIG                                    \
    {                                                       \
        .sck = BSP_SOFT_SPI_CLK_PIN,                          \
        .mosi = BSP_SOFT_SPI_MOSI_PIN,                        \
        .miso = BSP_SOFT_SPI_MISO_PIN,                        \
        .bus_name = "softspi",                                \
    }

struct k210_soft_spi_config
{
    rt_base_t sck;
    rt_base_t mosi;
    rt_base_t miso;
    const char *bus_name;
};

struct k210_soft_spi_device
{
    rt_base_t cs_pin;
    char *bus_name;
    char *device_name;
};

struct k210_soft_spi
{
    struct k210_soft_spi_config *config;
    struct rt_spi_configuration *cfg;
    struct rt_spi_bus spi_bus;
};







/* k210 soft spi dirver */




rt_err_t  rt_soft_spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cspin );
int rt_soft_spi_init(void);

#endif