#include <rtthread.h>
#include <rtdevice.h>
#include <drv_io_config.h>
#include <gpiohs.h>

#define UART1_NAME   "uart1"
static rt_device_t uart1_serial = RT_NULL;
static rt_thread_t uart1_can = RT_NULL;
static struct rt_semaphore rx_sem;
static struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */;
ALIGN(RT_ALIGN_SIZE)

/* 接收数据回调函数 */
static rt_err_t rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&rx_sem);
    return RT_EOK;
}

static void serial_thread_entry(void *parameter)
{
    char ch;
    while(1)
    {
        /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        while (rt_device_read(uart1_serial, 0, &ch, 1) == 1)
        {
            rt_kprintf("%02x ",ch);
        }
    }
}

static int uart1_thread(void)
{
    rt_err_t ret = RT_EOK;
    char uart_name[RT_NAME_MAX];
    rt_strncpy(uart_name, UART1_NAME, RT_NAME_MAX);

    /* 查找系统中的串口设备 */
    uart1_serial = rt_device_find(uart_name);
    if (!uart1_serial)
    {
        rt_kprintf("Find %s failed!\n", uart_name);
        return RT_ERROR;
    }
 
    /* 以中断接收及轮询发送模式打开串口设备 */
    ret = rt_device_open(uart1_serial, RT_DEVICE_FLAG_INT_RX);
    if (ret != RT_EOK)
    {
        rt_kprintf("Open %s device failed!\n", UART1_NAME);
        return ret;
    }

    /* 初始化信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(uart1_serial, rx_callback);

    /* 创建 uart1_can 测试线程 */
    uart1_can = rt_thread_create("uart1_can", serial_thread_entry, RT_NULL, 2048, 24, 5);
    if (uart1_can != RT_NULL)
    {
        rt_thread_startup(uart1_can);
    }
    else 
    {
        return RT_ERROR;
    }

    return ret;
}

static int can_cfg_start(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint8_t cmd[16];

    gpiohs_set_pin(FPIOA_CAN_NCFG, GPIO_PV_LOW);    //进入配置
    ret = rt_device_close(uart1_serial);    //先关闭
    rt_thread_mdelay(200);

    /* 修改串口配置参数 */
    config.baud_rate = BAUD_RATE_9600;        
    config.data_bits = DATA_BITS_8;         
    config.stop_bits = STOP_BITS_1;        
    config.bufsz     = 128;                  
    config.parity    = PARITY_NONE;       
    rt_device_control(uart1_serial, RT_DEVICE_CTRL_CONFIG, &config);

    /* 以波特率9600打开串口设备，切换到 配置模式*/
    ret = rt_device_open(uart1_serial, RT_DEVICE_FLAG_INT_RX);  
    if (ret != RT_EOK)
    {
        rt_kprintf("Open %s device failed!\n", UART1_NAME);
        return ret;
    }

    //初始化，设置参数
    gpiohs_set_drive_mode(FPIOA_CAN_NCFG, GPIO_DM_OUTPUT);
    gpiohs_set_pin(FPIOA_CAN_NCFG, GPIO_PV_LOW);
    rt_thread_mdelay(200);

    cmd[0] = 0xAA;  // 前两个字节固定为 0XAA 和 0X55,代表帧起始和帧同步
    cmd[1] = 0x55;
    cmd[2] = 0xFD;  // 0xFE:读配置  0xFD:写配置
    cmd[3] = 0x32;  // CAN参数 配置起始地址
    cmd[4] = 0x01;  // 数据长度：1字节
    cmd[5] = 0x0b;  // CAN 波特率=100K
    cmd[6] = 0xc4;  // CRC校验低字节
    cmd[7] = 0x29;  // CRC校验高字节
    rt_device_write(uart1_serial, 0, cmd, 8);
    rt_thread_mdelay(1000);

    // cmd[0] = 0xAA;  // 前两个字节固定为 0XAA 和 0X55,代表帧起始和帧同步
    // cmd[1] = 0x55;
    // cmd[2] = 0xFD;  // 0xFE:读配置  0xFD:写配置
    // cmd[3] = 0x02;  // 转换方式 配置起始地址
    // cmd[4] = 0x02;  // 数据长度：2字节
    // cmd[5] = 0x00;  // 双向透明转换
    // cmd[6] = 0x00;  // 转换方向（双向）
    // rt_device_write(uart1_serial, 0, cmd, 7);
    // rt_thread_mdelay(1000);

    return ret;
}

static int can_cfg_end(void)
{
    rt_err_t ret = RT_EOK;

    gpiohs_set_pin(FPIOA_CAN_NCFG, GPIO_PV_HIGH);    //手动退出配置
    ret = rt_device_close(uart1_serial);    //先关闭
    rt_thread_mdelay(200);

    /* 修改串口配置参数 */
    config.baud_rate = BAUD_RATE_115200;     
    config.data_bits = DATA_BITS_8;           
    config.stop_bits = STOP_BITS_1;          
    config.bufsz     = 128;                  
    config.parity    = PARITY_NONE;       
    rt_device_control(uart1_serial, RT_DEVICE_CTRL_CONFIG, &config);

    /* 重新以波特率115200打开串口设备，切换到 正常工作 模式*/
    ret = rt_device_open(uart1_serial, RT_DEVICE_FLAG_INT_RX);  
    if (ret != RT_EOK)
    {
        rt_kprintf("Open %s device failed!\n", UART1_NAME);
        return ret;
    }
    return ret;
}

static void can_test(void)
{
	rt_uint8_t msg[8];
	rt_uint8_t i;

    if (uart1_can == RT_NULL) 
    {   
        /* 1、启动线程 */
        uart1_thread();
        rt_thread_mdelay(20);

        /* 2、配置can参数 */
        can_cfg_start();
        rt_thread_mdelay(20);

        /* 3、手动退出配置 */
        can_cfg_end();
        rt_thread_mdelay(20);
    }

    /* 4、发送测试数据 */
    for(i=0;i<5;i++)
    {
        msg[0] = 0x11;
        msg[1] = 0x22;
        msg[2] = 0x33;
        msg[3] = 0x44;
        msg[4] = 0x55;
        msg[5] = 0x66;
        msg[6] = 0x77;
        msg[7] = 0x99;
        rt_device_write(uart1_serial, 0, msg, 8);
        rt_thread_mdelay(20);
    }
}
MSH_CMD_EXPORT(can_test, can send test );