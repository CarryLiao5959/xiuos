/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-05     Liuguang     the first version.
 */

#ifndef __DRV_LCD_H__
#define __DRV_LCD_H__

#include <rtthread.h>
#include <rtdevice.h>

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40
#define BRRED 0XFC07
#define GRAY 0X8430
#define GRAY175 0XAD75
#define GRAY151 0X94B2
#define GRAY187 0XBDD7
#define GRAY240 0XF79E
#define ORANGE 0XFD20


typedef struct 
{
    rt_uint16_t x_pos;
    rt_uint16_t y_pos;
    rt_uint16_t width;
    rt_uint16_t height;
    rt_uint8_t  font_size;
    rt_uint8_t *addr;
    rt_uint16_t font_color;
    rt_uint16_t back_color;
}lcdstringparam;

typedef struct 
{
    rt_uint16_t x_startpos;
    rt_uint16_t x_endpos;
    rt_uint16_t y_startpos;
    rt_uint16_t y_endpos;
    void* pixel_color;
}lcdpixelparam;

typedef struct 
{
    char type; // 0:write string;1:write dot
    lcdpixelparam pixel_info;
    lcdstringparam string_info;
}lcdwriteparam;

int rt_hw_lcd_init(void);

#endif
