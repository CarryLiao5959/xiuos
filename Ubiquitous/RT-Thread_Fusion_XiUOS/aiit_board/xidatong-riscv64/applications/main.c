/*
 * @Author: your name
 * @Date: 2021-10-11 22:04:25
 * @LastEditTime: 2021-10-14 11:12:52
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \xiuos\Ubiquitous\RT_Thread\bsp\k210\applications\main.c
 */
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

#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
//#define LED_G   12
extern int FrameworkInit(void);
int main(void)
{
    //rt_pin_mode(LED_G, PIN_MODE_OUTPUT);
    rt_thread_mdelay(100);
    char info1[25] ={0};
    char info2[25] ={0};
    sprintf(info1,"xuos-intelligence k210  ");
    sprintf(info2,"build %s %s",__DATE__,__TIME__);
    printf("%s %s \n",info1,info2); 
    FrameworkInit();
    #ifdef BSP_USING_LCD
    #include <drv_lcd.h>
    #include "drv_lt768_lcd.h"
    #include "LT768_Lib.h"
    static rt_device_t lcd_dev;
    // Accessing the lcd Device （the first step）
    lcd_dev = rt_device_find("lcd");
    if (!lcd_dev)
    {
        rt_kprintf("find %s failed!\n", "lcd");
        return RT_ERROR;
    }

    if (rt_device_open(lcd_dev,0) != RT_EOK)
    {
        lcd_dev = 0;
        rt_kprintf("open %s failed!\n", "lcd");
        return -RT_ERROR;
    }
    // lcd init
    LCD_init();

    LT768_DrawSquare_Fill(0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,White);
    rt_thread_mdelay(20);
    LT768_Select_Internal_Font_Init(32,1,1,1,1);
    LT768_Print_Internal_Font_String(0,30,RED,WHITE,"\\ | /");
    LT768_Print_Internal_Font_String(0,60,RED,WHITE,"- RT -");
    LT768_Print_Internal_Font_String(0,90,RED,WHITE,"/ | \\");
    LT768_Print_Internal_Font_String(0,120,RED,WHITE,info1);
    LT768_Print_Internal_Font_String(0,150,RED,WHITE,info2);
    #endif
    while(1)
    {
        //rt_pin_write(LED_G, PIN_HIGH);
        rt_thread_mdelay(500);
        //rt_pin_write(LED_G, PIN_LOW);
        //rt_thread_mdelay(500);
    }
    return 0;
}
