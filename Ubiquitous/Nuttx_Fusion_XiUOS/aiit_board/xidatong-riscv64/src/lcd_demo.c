/****************************************************************************
 * lcd_demo.c
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
* @file lcd_demo.c
* @brief
* @version 1.0.0
* @author AIIT XUOS Lab
* @date 2022-07-21
*/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "nuttx/arch.h"
#include "nuttx/lcd/lt768.h"
#include "nuttx/lcd/lt768_lib.h"
#include "nuttx/lcd/k210_lcd.h"

void LcdDemo(void)
{
//    int x1 = 0, y1 = 0, x2 = LCD_XSIZE_TFT, y2 = LCD_YSIZE_TFT;
    int x1 = 100, y1 = 100, x2 = 200, y2 = 200;

    Main_Image_Start_Address(LCD_START_ADDR);
    Main_Image_Width(LCD_XSIZE_TFT);
    Main_Window_Start_XY(0, 0);
    Canvas_Image_Start_address(LCD_START_ADDR);
    Canvas_image_width(LCD_XSIZE_TFT);
    Active_Window_XY(0, 0);
    Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT);
    up_mdelay(10);
    Canvas_Image_Start_address(LCD_START_ADDR);

    for(int i = 0; i < 3; i++)
    {
        syslog(LOG_NOTICE, "Disp_demo %d\n", i);
        LT768_DrawSquare_Fill(x1, y1, x2, y2, Red);
        up_mdelay(2000);
        LT768_DrawSquare_Fill(x1, y1, x2, y2, Green);
        up_mdelay(2000);
        LT768_DrawSquare_Fill(x1, y1, x2, y2, Blue);
        up_mdelay(2000);
    }
}

