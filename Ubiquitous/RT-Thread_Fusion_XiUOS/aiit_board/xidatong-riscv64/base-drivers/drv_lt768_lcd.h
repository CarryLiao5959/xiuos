/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-07     ZYH          first version
 */

#ifndef DL_DRV_LCD_H__
#define DL_DRV_LCD_H__

#include <rtthread.h>
#include "LT768.h"
#include "LT768_Lib.h"
#include "drv_lt768_lcd.h"

#define Picture_1_Addr  0                                // Image 1 addresses in FLASH

#define layer1_start_addr 0                                // The layer is in the video memory position

#define Resolution  (LCD_XSIZE_TFT*LCD_YSIZE_TFT)   

void LCD_init(void);

rt_uint8_t LCD_Read_Byte(void);
void LCD_Send_Byte(rt_uint8_t dt);
rt_uint8_t LCD_ReadWriteByte(rt_uint8_t TxData);

void LCD_CmdWrite(rt_uint8_t cmd);
void LCD_DataWrite(rt_uint8_t data);
void LCD_DataWrite_Pixel(rt_uint8_t data);
rt_uint8_t LCD_StatusRead(void);
rt_uint8_t LCD_DataRead(void);
void LCD_udelay(rt_uint64_t usec);

#endif
