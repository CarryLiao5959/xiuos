#include <rtthread.h>
#ifdef BSP_USING_LCD_XIDATONG
#include <drv_lcd.h>
static rt_device_t dev = 0;
static struct rt_device_graphic_info info;
static int lcd_init(void)
{
    int result;

    if (dev)
        return 0;

    dev = rt_device_find("lcd");
    if (!dev)
        return -1;

    if (rt_device_open(dev, 0) != 0)
    {
        dev = 0;
        return -1;
    }

    result = rt_device_control(dev, RTGRAPHIC_CTRL_GET_INFO, &info);
    if (result != 0)
    {
        rt_device_close(dev);
        dev = 0;
        return result;
    }
    return 0;
}
INIT_ENV_EXPORT(lcd_init);

static void lcd_clear(rt_uint16_t color)
{
    for(int i = 0;i < info.height*info.width;i++)
    {
        *((rt_uint16_t*)info.framebuffer+i) = color;
    }
}

void lcd_test()
{
    lcd_clear(BLUE);
    rt_thread_mdelay(1000);
    lcd_clear(GREEN);
    rt_thread_mdelay(1000);
    lcd_clear(GRAY);
    rt_thread_mdelay(1000);
    lcd_clear(RED);
}
MSH_CMD_EXPORT(lcd_test,lcd test);

#endif