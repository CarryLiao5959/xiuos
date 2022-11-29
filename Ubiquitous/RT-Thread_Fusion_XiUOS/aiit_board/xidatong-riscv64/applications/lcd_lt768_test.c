#include <rtthread.h>
#include <drv_lcd.h>
#include <drv_lt768_lcd.h>
#include <LT768_Lib.h>

static int lcd_test(void)
{
    char *devname = "lcd";
    rt_device_t dev;
    dev = rt_device_find(devname);
    if (!dev) {
        rt_kprintf("lcd: %s not found\n", devname);
        return -1;
    }

    if (rt_device_open(dev, 0) != 0) {
        rt_kprintf("lcd open fail\n");
        return -1;
    }

    // lcd init
    LCD_init();  // If it's not initialized in main, it needs to be turned on here.

    rt_kprintf("Display background color.\n");
    LT768_DrawSquare_Fill(0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,Red);
    rt_thread_mdelay(1000);
    LT768_DrawSquare_Fill(0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,Green);
    rt_thread_mdelay(1000);
    LT768_DrawSquare_Fill(0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,Blue);
    rt_thread_mdelay(1000);

    rt_kprintf("Display text.\n");
    LT768_DrawSquare_Fill(0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,White);
    LT768_Select_Internal_Font_Init(32,1,1,1,1);
    LT768_Print_Internal_Font_String(0,30,RED,WHITE,"LCD Testing");
    LT768_Print_Internal_Font_String(0,90,RED,WHITE,"AIIT k210");
    rt_thread_mdelay(1000);

    rt_kprintf("Display graphics.\n");
    LT768_DrawSquare_Fill(0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,White);
    LT768_DrawCircle_Fill(100,135,50,Red);
    rt_thread_mdelay(1000);
    LT768_DrawTriangle_Fill(200,160,260,160,230,95,Cyan);
    rt_thread_mdelay(1000);
    LT768_DrawCylinder(370,150,50,30,50,Black,Yellow);
    rt_thread_mdelay(1000);

    return 0;
}
MSH_CMD_EXPORT(lcd_test, A LCD test sample);
