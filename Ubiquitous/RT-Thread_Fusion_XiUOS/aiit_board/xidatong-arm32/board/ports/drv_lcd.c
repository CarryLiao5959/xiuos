/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-10-30     Tanek        the first version
 * 2018-04-05     Liu2guang    export LCD config parameters.
 */

#include <rtthread.h>

#ifdef BSP_USING_LCD_XIDATONG
#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#ifdef RT_USING_POSIX_TERMIOS
#include <posix_termios.h>
#endif
#endif
#include "drv_lcd.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_elcdif.h"
#include "drv_gpio.h"
#if !defined(LCD_WIDTH) || !defined(LCD_HEIGHT)
#error "Please config lcd pixel parameters."
#endif

#if !defined(LCD_HFP) || !defined(LCD_HBP) || !defined(LCD_HSW) || \
    !defined(LCD_VFP) || !defined(LCD_VBP) || !defined(LCD_VSW)
#error "Please config lcd timing parameters."
#endif

#if !defined(LCD_BL_PIN) || !defined(LCD_RST_PIN)
#error "Please config lcd backlight or reset pin."
#endif
#define DRV_DEBUG
#define LOG_TAG             "drv.lcd"
#define DBG_LVL               DBG_LOG
#include <rtdbg.h>

struct imxrt_lcd
{
    struct rt_device device;
    struct rt_device_graphic_info info;
};

static struct imxrt_lcd lcd;
AT_NONCACHEABLE_SECTION_ALIGN(static uint16_t frame_buffer[LCD_HEIGHT][LCD_WIDTH], 64);

static rt_err_t imxrt_lcd_init(rt_device_t device)
{
    RT_ASSERT(device != RT_NULL);

    rt_memset(frame_buffer, 0x00, sizeof(frame_buffer));
    clock_video_pll_config_t pll_config;

    pll_config.loopDivider = 43;
    pll_config.postDivider = 4;
    pll_config.numerator   = 0;
    pll_config.denominator = 0;

    CLOCK_InitVideoPll(&pll_config);

    rt_pin_mode(LCD_RST_PIN, PIN_MODE_OUTPUT); /* LCD_RESET */
    rt_pin_write(LCD_RST_PIN, PIN_LOW);
    rt_thread_delay(RT_TICK_PER_SECOND / 100);
    rt_pin_write(LCD_RST_PIN, PIN_HIGH);

    rt_pin_mode (LCD_BL_PIN, PIN_MODE_OUTPUT);  /* LCD_BL */
    rt_pin_write(LCD_BL_PIN, PIN_HIGH);

    /* LCD */
    elcdif_rgb_mode_config_t lcd_config;

    lcd_config.hfp           = LCD_HFP;
    lcd_config.vfp           = LCD_VFP;
    lcd_config.hbp           = LCD_HBP;
    lcd_config.vbp           = LCD_VBP;
    lcd_config.hsw           = LCD_HSW;
    lcd_config.vsw           = LCD_VSW;

    lcd_config.polarityFlags = kELCDIF_DataEnableActiveHigh |
                               kELCDIF_VsyncActiveHigh      |
                               kELCDIF_HsyncActiveLow       |
                               kELCDIF_DriveDataOnRisingClkEdge;

    lcd_config.panelWidth    = LCD_WIDTH;
    lcd_config.panelHeight   = LCD_HEIGHT;
    lcd_config.pixelFormat   = kELCDIF_PixelFormatRGB565;
    lcd_config.dataBus       = kELCDIF_DataBus16Bit;
    lcd_config.bufferAddr    = (uint32_t)frame_buffer;
    ELCDIF_RgbModeInit (LCDIF, &lcd_config);
    ELCDIF_RgbModeStart(LCDIF);

    /* LCD DEVICE */
    lcd.info.width          = LCD_WIDTH;
    lcd.info.height         = LCD_HEIGHT;
    lcd.info.pixel_format   = RTGRAPHIC_PIXEL_FORMAT_RGB565;
    lcd.info.bits_per_pixel = 16;
    lcd.info.framebuffer    = (void *)frame_buffer;
    LOG_I("imxrt lcd init success!!\n\r");
    return RT_EOK;
}

static rt_err_t imxrt_lcd_control(rt_device_t device, int cmd, void *args)
{
    switch(cmd)
    {
    case RTGRAPHIC_CTRL_RECT_UPDATE:
        break;

    case RTGRAPHIC_CTRL_POWERON:
        rt_pin_write(LCD_BL_PIN, PIN_HIGH);
        break;

    case RTGRAPHIC_CTRL_POWEROFF:
        rt_pin_write(LCD_BL_PIN, PIN_LOW);
        break;

    case RTGRAPHIC_CTRL_GET_INFO:
        rt_memcpy(args, &lcd.info, sizeof(lcd.info));
        break;

    case RTGRAPHIC_CTRL_SET_MODE:
        break;
    }

    return RT_EOK;
}


static void drv_lcd_set_pixel_dot(rt_uint16_t x1, rt_uint16_t y1, rt_uint16_t x2, rt_uint16_t y2, void* color)
{
    rt_uint16_t i = 0;
    rt_uint16_t j = 0;

    for(i = y1; i <= y2; i++) 
    {
        for(j = x1; j <= x2; j++) 
        {
            frame_buffer[i][j] =(*(rt_uint16_t*)color);
            (rt_uint16_t*)color += 1;
        }
    }
}




#ifdef RT_USING_POSIX
static int lcd_fops_open(struct dfs_fd *fd)
{
    rt_err_t ret = RT_EOK;
    rt_uint16_t flags = 0;
    rt_device_t device;
    device = (rt_device_t)fd->data;
    RT_ASSERT(device != RT_NULL);
    flags = RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_RDWR;
    ret = rt_device_open(device, flags);
    if (ret == RT_EOK) 
    {
        return RT_EOK;
    }
    else
    {
        return ret;
    }
}

static int lcd_fops_close(struct dfs_fd *fd)
{
    rt_device_t device;
    device = (rt_device_t)fd->data;
    rt_device_close(device);
    return RT_EOK;
}

static int lcd_fops_ioctl(struct dfs_fd *fd, int cmd, void *args)
{
    rt_device_t device;
    device = (rt_device_t)fd->data;
    return rt_device_control(device, cmd, args);
}

static int lcd_fops_read(struct dfs_fd *fd, void *buf, size_t count)
{
    return count;
}

static int lcd_fops_write(struct dfs_fd *fd, const void *buf, size_t count)
{
    rt_err_t ret = RT_EOK;
    if (buf) 
    {
        lcdwriteparam *show = (lcdwriteparam *)buf;
        //output string
        if(0 == show->type) 
        {
            
        } 
        //output dot
        else if (1 == show->type) 
        {
            drv_lcd_set_pixel_dot(show->pixel_info.x_startpos,show->pixel_info.y_startpos, show->pixel_info.x_endpos, show->pixel_info.y_endpos,show->pixel_info.pixel_color);
        } else 
        {
            rt_kprintf("LcdWrite donnot support show type(0 string; 1 dot) %u\n", show->type);
            ret = -RT_ERROR;
        }
    }
    
    return ret; 
}
const static struct dfs_file_ops lcd_fops =
{
    lcd_fops_open,
    lcd_fops_close,
    lcd_fops_ioctl,
    lcd_fops_read,
    lcd_fops_write,
    RT_NULL, /* flush */
    RT_NULL, /* lseek */
    RT_NULL, /* getdents */
    RT_NULL,/*poll*/
};
#endif

int rt_hw_lcd_init(void)
{
    rt_err_t ret;

    lcd.device.type    = RT_Device_Class_Graphic;
    lcd.device.init    = imxrt_lcd_init;
    lcd.device.open    = RT_NULL;
    lcd.device.close   = RT_NULL;
    lcd.device.read    = RT_NULL;
    lcd.device.write   = RT_NULL;
    lcd.device.control = imxrt_lcd_control;

    lcd.device.user_data = (void *)&lcd.info;

    ret = rt_device_register(&lcd.device, "lcd", RT_DEVICE_FLAG_RDWR);
    #ifdef RT_USING_POSIX
    lcd.device.fops = &lcd_fops;
    #endif
    return ret;
}

INIT_DEVICE_EXPORT(rt_hw_lcd_init);

#endif /* BSP_USING_LCD */
