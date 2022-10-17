/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-12     ZYH          first version
 */


#include <rtthread.h>
#include "LT768.h"
#include "LT768_Lib.h"
#include <gpiohs.h>
#include "drv_io_config.h"
#include "drv_lt768_lcd.h"
#include "sleep.h"


#define LCD_XSIZE_TFT   480
#define LCD_YSIZE_TFT   272
#define LCD_DEVICE_NAME "lcd"


void LCD_init(void)
{   
    rt_uint8_t i;
    rt_uint8_t PwmControl = 100;
    
    gpiohs_set_drive_mode(FPIOA_LCD_MISO, GPIO_DM_INPUT);
    gpiohs_set_drive_mode(FPIOA_LCD_NRST, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(FPIOA_LCD_SCLK, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(FPIOA_LCD_MOSI, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(FPIOA_LCD_NCS, GPIO_DM_OUTPUT);

    // LT768 init
    LT768_HW_Reset();
    LT768_Init();
    
    Select_SFI_Dual_Mode0();
    LT768_PWM1_Init(1,0,200,100,PwmControl);    // PWM1 Enables the backlight
    Display_ON();                               // Enable RGB output
//  Waiting_Key();                              // Waiting for the button
//  Main_GUI();                                 // Display the main menu
    Main_Image_Start_Address(layer1_start_addr);
    Main_Image_Width(LCD_XSIZE_TFT);
    Main_Window_Start_XY(0,0);
    Canvas_Image_Start_address(layer1_start_addr);
    Canvas_image_width(LCD_XSIZE_TFT);
    Active_Window_XY(0,0);
    Active_Window_WH(LCD_XSIZE_TFT,LCD_YSIZE_TFT);
    rt_thread_mdelay(10);
    Canvas_Image_Start_address(layer1_start_addr);
//    LT768_DMA_24bit_Block(1,0,0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,LCD_XSIZE_TFT,0x0014C000);
}

rt_uint8_t LCD_Read_Byte(void)
{
    rt_uint8_t i,rByte=0;

    gpiohs_set_pin(FPIOA_LCD_SCLK, GPIO_PV_HIGH);    usleep(10);//usleep(1);
    for(i=0;i<8;i++)
    {
        gpiohs_set_pin(FPIOA_LCD_SCLK, GPIO_PV_LOW);   usleep(10);//usleep(1);
        rByte<<=1;
        rByte|=gpiohs_get_pin(FPIOA_LCD_MISO);      usleep(10);//usleep(1);
        gpiohs_set_pin(FPIOA_LCD_SCLK, GPIO_PV_HIGH);    usleep(10);//usleep(1);
    }

    return rByte;
}

void LCD_Send_Byte(rt_uint8_t dt)
{
    rt_uint8_t i;

    for(i=0;i<8;i++)
    {
        gpiohs_set_pin(FPIOA_LCD_SCLK, GPIO_PV_HIGH);    usleep(10);//usleep(1);
        if((dt<<i)&0x80)
            gpiohs_set_pin(FPIOA_LCD_MOSI, GPIO_PV_HIGH);
        else
            gpiohs_set_pin(FPIOA_LCD_MOSI, GPIO_PV_LOW);
        usleep(10);//usleep(1);
        gpiohs_set_pin(FPIOA_LCD_SCLK, GPIO_PV_LOW);   usleep(10);//usleep(1);
    }
    gpiohs_set_pin(FPIOA_LCD_SCLK, GPIO_PV_HIGH);    usleep(10);//usleep(1);

}

rt_uint8_t LCD_ReadWriteByte(rt_uint8_t TxData)
{
//    LCD_Send_Byte(TxData);
//    return LCD_Read_Byte();               

    rt_uint8_t i,rByte=0;

    for(i=0;i<8;i++)
    {
        gpiohs_set_pin(FPIOA_LCD_SCLK, GPIO_PV_HIGH);    usleep(10);//usleep(1);
        if((TxData<<i)&0x80)
            gpiohs_set_pin(FPIOA_LCD_MOSI, GPIO_PV_HIGH);
        else
            gpiohs_set_pin(FPIOA_LCD_MOSI, GPIO_PV_LOW);
        usleep(10);//usleep(1);
        gpiohs_set_pin(FPIOA_LCD_SCLK, GPIO_PV_LOW);   usleep(10);//usleep(1);

        rByte<<=1;
        rByte|=gpiohs_get_pin(FPIOA_LCD_MISO);
    }
    gpiohs_set_pin(FPIOA_LCD_SCLK, GPIO_PV_HIGH);    usleep(10);//usleep(1);

    return rByte;
}

void LCD_CmdWrite(rt_uint8_t cmd)
{
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_LOW);    usleep(10);//usleep(1);
    LCD_ReadWriteByte(0x00);
    LCD_ReadWriteByte(cmd);
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_HIGH);    usleep(10);//usleep(1);
}

void LCD_DataWrite(rt_uint8_t data)
{
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_LOW);    usleep(10);//usleep(1);
    LCD_ReadWriteByte(0x80);
    LCD_ReadWriteByte(data);
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_HIGH);    usleep(10);//usleep(1);
}

void LCD_DataWrite_Pixel(rt_uint8_t data)
{
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_LOW);    usleep(10);//usleep(1);
    LCD_ReadWriteByte(0x80);
    LCD_ReadWriteByte(data);
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_HIGH);    usleep(10);//usleep(1);

    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_LOW);    usleep(10);//usleep(1);
    LCD_ReadWriteByte(0x80);
    LCD_ReadWriteByte(data>>8);
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_HIGH);    usleep(10);//usleep(1);
}

rt_uint8_t LCD_StatusRead(void)
{
    rt_uint8_t temp = 0;
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_LOW);    usleep(10);//usleep(1);
    LCD_ReadWriteByte(0x40);
    temp = LCD_ReadWriteByte(0xff);
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_HIGH);    usleep(10);//usleep(1);
    rt_kprintf("%s temp = 0x%x\n",__func__,temp);
    return temp;
}

rt_uint8_t LCD_DataRead(void)
{
    rt_uint8_t temp = 0;
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_LOW);    usleep(10);//usleep(1);
    LCD_ReadWriteByte(0xc0);
    temp = LCD_ReadWriteByte(0xff);
    gpiohs_set_pin(FPIOA_LCD_NCS, GPIO_PV_HIGH);    usleep(10);//usleep(1);
    rt_kprintf("%s temp = 0x%x\n",__func__,temp);
    return temp;
}
