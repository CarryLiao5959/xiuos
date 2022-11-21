#include <rtthread.h>
#include <touch.h>
#include "drv_touch.h"
#ifdef BSP_USING_I2C1
rt_uint8_t CTP_CFG_GT911[] =  {
    0x5B,0xE0,0x01,0x10,0x01,0x0A,0x0D,0x00,0x01,0x0A,
    0x28,0x0F,0x5A,0x3C,0x03,0x05,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x87,0x28,0x09,
    0x32,0x34,0x0C,0x08,0x00,0x00,0x00,0x02,0x02,0x1D,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x28,0x55,0x94,0xC5,0x02,0x07,0x00,0x00,0x04,
    0x8D,0x2B,0x00,0x80,0x32,0x00,0x75,0x3A,0x00,0x6C,
    0x43,0x00,0x64,0x4F,0x00,0x64,0x00,0x00,0x00,0x00,
    0xF0,0x4A,0x3A,0xFF,0xFF,0x27,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,
    0x14,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x26,0x24,0x22,0x21,0x20,0x1F,0x1E,0x1D,
    0x0C,0x0A,0x08,0x06,0x04,0x02,0x00,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x81,0x01
};
rt_uint8_t config[GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH]
                = {GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff};

void  ok1052_touch_init(struct rt_touch_device *touch)
{
    rt_int32_t ret = -1;
    rt_int32_t i = 0;
    rt_uint8_t check_sum = 0;
    rt_int32_t retry = 0;
    rt_uint8_t cfg_num =0x80FE - 0x8047+1 ;
    struct rt_i2c_msg msgs_w;
    const rt_uint8_t* cfg_info = CTP_CFG_GT911;
	
    rt_uint8_t cfg_info_len = CFG_GROUP_LEN(CTP_CFG_GT911) ;

    rt_memset(&config[GTP_ADDR_LENGTH], 0, GTP_CONFIG_MAX_LENGTH);
    rt_memcpy(&config[GTP_ADDR_LENGTH], cfg_info,cfg_info_len);
    rt_ok1052_t dev = (rt_ok1052_t)touch;
    check_sum = 0;
    for (i = GTP_ADDR_LENGTH; i < cfg_num+GTP_ADDR_LENGTH; i++)
    {
        check_sum += config[i];
    }
    config[ cfg_num+GTP_ADDR_LENGTH] = (~check_sum) + 1; 	
    config[ cfg_num+GTP_ADDR_LENGTH+1] =  1; 				

    msgs_w.addr = RT_GTP_ADDRESS;
    msgs_w.flags = RT_I2C_WR;
    msgs_w.len = cfg_num + GTP_ADDR_LENGTH+2;
    msgs_w.buf = config;

    ret = rt_i2c_transfer(dev->i2c,&msgs_w,1);
    if (ret < 0)
    {
        rt_kprintf("I2C write end_cmd error!\n");
        ret = 0;
    }
}

TouchDataStandard_drv Pre_Touch_Point = {0};
static rt_size_t ok1052_touch_readpoint(struct rt_touch_device *touch, void *buf, rt_size_t touch_num)
{
	int ret = 0;
    static int int_time = 0;
    if(0 == int_time)
    {
        ok1052_touch_init(touch);
        int_time =1;
    }
    rt_uint8_t  end_cmd[3] = { GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF, 0 };
    rt_uint8_t  point_data[2 + 1 + 8 * GTP_MAX_TOUCH + 1]={GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF};
    rt_uint8_t retries = 0;
    rt_uint8_t  finger = 0;
    rt_uint8_t touch_num1 = 0;
    static rt_uint16_t pre_touch = 0;
    rt_uint8_t* coor_data = NULL;
    if (touch_num == 0 || touch_num > GTP_MAX_TOUCH)
    {
        return 0;
    }
    TouchDataStandard_drv *result = (TouchDataStandard_drv *)buf;
    rt_ok1052_t dev = (rt_ok1052_t)touch;
    struct rt_i2c_msg msgs[2];
    struct rt_i2c_msg msgs_w;
    msgs[0].addr = RT_GTP_ADDRESS;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].len = GTP_ADDR_LENGTH;
    msgs[0].buf = &point_data[0];

    msgs[1].addr = RT_GTP_ADDRESS;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].len = 12-GTP_ADDR_LENGTH;
    msgs[1].buf = &point_data[GTP_ADDR_LENGTH];
    while(retries < 5)
    {
        if(2 != rt_i2c_transfer(dev->i2c,msgs,2))
        {
            retries++;
            if(retries >= 5)
            {
                return  -RT_ERROR;
            }
        }
        else
        {
            break;
        }
    }
    finger = point_data[GTP_ADDR_LENGTH];//状态寄存器数据
    if (finger == 0x00)		//没有数据，退出
    {
        ret = 0;
        goto exit_work_func;
    }
    if((finger & 0x80) == 0)//判断buffer status位
    {
        ret = 0;
        goto exit_work_func;//坐标未就绪，数据无效
    }
    touch_num1 = finger & 0x0f;//坐标点数
    if(touch_num1 > GTP_MAX_TOUCH)
    {
        ret = 0;
        goto exit_work_func;//大于最大支持点数，错误退出
    }
    if (touch_num1)
    {
        coor_data = &point_data[0 * 8 + 3];
        result->x  = coor_data[1] | (coor_data[2] << 8);	//x坐标
        result->y  = coor_data[3] | (coor_data[4] << 8);	//y坐标
    }
    else if(pre_touch)
    {
        *result = Pre_Touch_Point;
        result->x  = -1;
        result->y  = -1;
    }
    pre_touch = touch_num1;
exit_work_func:
    {
        msgs_w.addr = RT_GTP_ADDRESS;
        msgs_w.flags = RT_I2C_WR;
        msgs_w.len = 3;
        msgs_w.buf = &end_cmd[0];

        ret = rt_i2c_transfer(dev->i2c,&msgs_w,1);
        if (ret < 0)
        {
            rt_kprintf("I2C write end_cmd error!\n");
            ret = 0;
        }
    }
	return ret;

}

static rt_size_t ok1052_touch_control(struct rt_touch_device *touch, int cmd, void *arg)
{
    rt_err_t result = RT_EOK;
    RT_ASSERT(touch != RT_NULL);

    /* If necessary, please implement this control function yourself */

    return result;
}

static struct rt_touch_ops ok1052_ops =
{
    .touch_readpoint = ok1052_touch_readpoint,
    .touch_control = ok1052_touch_control,
};

static int ok1052_hw_init(void)
{
    rt_ok1052_t dev_obj = rt_malloc(sizeof(struct rt_ok1052));
    if (dev_obj != RT_NULL)
    {
        rt_memset(dev_obj, 0x0, sizeof(struct rt_ok1052));
        dev_obj->i2c= (struct rt_i2c_bus_device *)rt_device_find(OK1052_TOUCH_IIC_NAME);
        RT_ASSERT(dev_obj->i2c != RT_NULL );
        dev_obj->parent.info.type = RT_TOUCH_TYPE_RESISTANCE;
        dev_obj->parent.info.vendor = RT_TOUCH_VENDOR_UNKNOWN;
        dev_obj->parent.info.point_num = GTP_MAX_TOUCH;
        dev_obj->parent.info.range_x = GTP_MAX_WIDTH;
        dev_obj->parent.info.range_y = GTP_MAX_HEIGHT;
#ifdef RT_TOUCH_PIN_IRQ
        dev_obj->parent.config.irq_pin.pin = BSP_OK1052_IRQ_PIN;
        dev_obj->parent.config.irq_pin.mode = PIN_MODE_INPUT;
        
#endif /* RT_TOUCH_PIN_IRQ */
        dev_obj->parent.ops = &ok1052_ops;
        rt_hw_touch_register(&(dev_obj->parent), OK1052_TOUCH_NAME, RT_DEVICE_FLAG_INT_RX, RT_NULL);
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

INIT_APP_EXPORT(ok1052_hw_init);
#endif