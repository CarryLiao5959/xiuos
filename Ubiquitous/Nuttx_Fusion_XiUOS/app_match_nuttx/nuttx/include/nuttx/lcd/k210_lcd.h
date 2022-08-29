/****************************************************************************
 * k210_lcd.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/**
* @file k210_lcd.h
* @brief
* @version 1.0.0
* @author AIIT XUOS Lab
* @date 2022-07-21
*/

#ifndef __K210_LCD_H_
#define __K210_LCD_H_

#include <stdbool.h>

#include <debug.h>
#include <syslog.h>

#include "nuttx/lcd/lt768.h"
#include "nuttx/lcd/lt768_lib.h"

//PIN.define
#define BSP_LCD_NRST        37
#define BSP_LCD_SCLK        38
#define BSP_LCD_MOSI        39
#define BSP_LCD_MISO        40
#define BSP_LCD_NCS         41
#define BSP_LCD_BL_PIN      47

//FPIOA.define
#define FPIOA_LCD_NRST      0
#define FPIOA_LCD_BL        9
#define FPIOA_LCD_SCLK      28
#define FPIOA_LCD_MOSI      29
#define FPIOA_LCD_MISO      23 //can't use GPIOHS30
#define FPIOA_LCD_NCS       31

#define GPIO_PV_HIGH 1 // true
#define GPIO_PV_LOW 0  // false

#define LCD_START_ADDR 0
#define STM32_FSMC_8 0

#define lcd_set_pin k210_gpiohs_set_value
#define lcd_get_pin k210_gpiohs_get_value

void test_delay(void);

#define lcd_delay() up_udelay(20)

#define NCS_H lcd_set_pin(FPIOA_LCD_NCS, GPIO_PV_HIGH); lcd_delay()
#define NCS_L lcd_set_pin(FPIOA_LCD_NCS, GPIO_PV_LOW); lcd_delay()

#define CLK_H lcd_set_pin(FPIOA_LCD_SCLK, GPIO_PV_HIGH); lcd_delay()
#define CLK_L lcd_set_pin(FPIOA_LCD_SCLK, GPIO_PV_LOW); lcd_delay()

#define MOSI_H lcd_set_pin(FPIOA_LCD_MOSI, GPIO_PV_HIGH)
#define MOSI_L lcd_set_pin(FPIOA_LCD_MOSI, GPIO_PV_LOW)

/*----------------------------------------------------------------------------*/

void lcd_backlight_init(bool enable);
void lcd_drv_init(void);

void LCD_CmdWrite(uint8_t cmd);
void LCD_DataWrite(uint8_t data);
uint8_t LCD_DataRead(void);
uint8_t LCD_StatusRead(void);
void LCD_DataWrite_Pixel(uint8_t data);

/*----------------------------------------------------------------------------*/

#endif
