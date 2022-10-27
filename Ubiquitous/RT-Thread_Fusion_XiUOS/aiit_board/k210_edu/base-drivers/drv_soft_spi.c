/*
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-19     xiaoyu     first version
 */

#include "board.h"
#ifdef  BSP_USING_SOFT_SPI
#include <drv_soft_spi.h>
#include <gpiohs.h>
#include <fpioa.h>
#include <sleep.h>
#include "gpio.h"
#include <sysctl.h>

static struct k210_soft_spi_config soft_spi_config[] ={
    SOFT_SPI_BUS_CONFIG,
};

static struct k210_soft_spi soft_spi_bus_obj[sizeof(soft_spi_config) / sizeof(soft_spi_config[0])] = {0};

static rt_err_t k210_spi_init(struct k210_soft_spi *spi_drv, struct rt_spi_configuration *cfg)
{
    RT_ASSERT(spi_drv != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

	//mode  CPOL = 0  CPHA = 0 
    rt_pin_mode(spi_drv->config->sck, PIN_MODE_OUTPUT);
    rt_pin_mode(spi_drv->config->mosi, PIN_MODE_OUTPUT);
    rt_pin_mode(spi_drv->config->miso, PIN_MODE_INPUT);
    rt_pin_write(spi_drv->config->sck,PIN_LOW);
    rt_kprintf("%s init done", spi_drv->config->bus_name);
    return RT_EOK;
}
/*
delay us
*/
void spi_delay(uint64_t usec)
{
    if(usec <= 0)
    {
        return;
    }
    uint64_t cycle = read_cycle();
    uint64_t nop_all = usec * sysctl_clock_get_freq(SYSCTL_CLOCK_CPU) / 1000000UL;
    while (1)
    {
        if(read_cycle() - cycle >= nop_all)
            break;
    }
    return;
}

static void soft_spi_writebyte(struct k210_soft_spi *spi_drv,uint8_t data)
{
    int8_t i = 0;
    uint8_t temp = 0;
    for(i=0; i<8; i++) 
    {
        temp = ((data&0x80)==0x80)? 1:0;
        data = data<<1;
        rt_pin_write(spi_drv->config->sck,PIN_LOW);
        if(temp == 0)
        {
            rt_pin_write(spi_drv->config->mosi,PIN_LOW);
        }
        else
        {
            rt_pin_write(spi_drv->config->mosi,PIN_HIGH);
        }
        spi_delay(0);
        rt_pin_write(spi_drv->config->sck,PIN_HIGH);
        spi_delay(0); 
    }
    rt_pin_write(spi_drv->config->sck,PIN_LOW); 
}


/* 读一个字节 */
static uint8_t soft_spi_readbyte(struct k210_soft_spi *spi_drv)
{
    uint8_t i = 0;
    uint8_t read_data = 0xFF;
    for(i=0; i<8; i++) 
    {
        read_data = read_data << 1;
        rt_pin_write(spi_drv->config->sck,PIN_LOW);
        spi_delay(0);
        rt_pin_write(spi_drv->config->sck,PIN_HIGH);
        spi_delay(0);
        if(rt_pin_read(spi_drv->config->miso)==1) 
        {
            read_data = read_data + 1; 
        } 
    }
    rt_pin_write(spi_drv->config->sck,PIN_LOW);
    return read_data;
}


/* 读写一个字节 */
static uint8_t soft_spi_readwritebyte(struct k210_soft_spi *spi_drv,uint8_t data)
{
    uint8_t i = 0;
    uint8_t temp = 0;
    uint8_t read_data = 0xFF;
    for(i=0;i<8;i++) 
    {
        temp = ((data&0x80)==0x80)? 1:0;
        data = data<<1;
        read_data = read_data<<1;
        rt_pin_write(spi_drv->config->sck,PIN_LOW);
        if(temp == 0)
        {
            rt_pin_write(spi_drv->config->mosi,PIN_LOW);
        }
        else
        {
            rt_pin_write(spi_drv->config->mosi,PIN_HIGH);
        }
        spi_delay(0);
        rt_pin_write(spi_drv->config->sck,PIN_HIGH);
        spi_delay(0);
        if(rt_pin_read(spi_drv->config->miso)==1) 
        {
            read_data = read_data + 1; 
        } 
        }
        rt_pin_write(spi_drv->config->sck,PIN_LOW);
        return read_data;
}

static rt_uint32_t spixfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    rt_uint32_t state;
    rt_size_t message_length;
    rt_uint8_t *recv_buf;
    const rt_uint8_t *send_buf;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    RT_ASSERT(device->bus->parent.user_data != RT_NULL);
    RT_ASSERT(message != RT_NULL);

    struct k210_soft_spi *spi_drv =  rt_container_of(device->bus, struct k210_soft_spi, spi_bus);
    rt_base_t  *cs = device->parent.user_data;
    rt_uint32_t size = message->length;
    if (message->cs_take)
    {
         rt_pin_write(*cs,PIN_LOW);
    }
    const rt_uint8_t * send_ptr =  message->send_buf;
    rt_uint8_t * recv_ptr =  message->recv_buf;
    rt_uint8_t data = 0xFF;
    rt_uint8_t rxdata = 0xFF;
    while(size)
    {
        if((send_ptr != RT_NULL)&&(recv_ptr != RT_NULL))
        {
            for(int i =0; i< size;i++)
            {
                data = *(send_ptr+i);
                rxdata = soft_spi_readwritebyte(spi_drv,data);
                *(recv_ptr+i) = rxdata;
            }
            break;

        }
        else if(send_ptr != RT_NULL)
        {
            for(int i =0; i< size;i++)
            {
                data = *(send_ptr+i);
                soft_spi_writebyte(spi_drv,data);
            }
            break;
        }
        else 
        {
            for(int i =0; i< size;i++)
            {
                data = soft_spi_readbyte(spi_drv);
                *(recv_ptr+i) = data;
            }
            break;
        }        
    }
    if(message->cs_release)
    {
         rt_pin_write(*cs,PIN_HIGH);
    }

    return message->length;
}

static rt_err_t spi_configure(struct rt_spi_device *device,struct rt_spi_configuration *configuration){
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(configuration != RT_NULL);

    struct k210_soft_spi *spi_drv =  rt_container_of(device->bus, struct k210_soft_spi, spi_bus);
    spi_drv->cfg = configuration;

    return k210_spi_init(spi_drv, configuration);
}

static const struct rt_spi_ops stm_spi_ops ={
    .configure = spi_configure,
    .xfer = spixfer,
};

static int rt_soft_spi_bus_init(void)
{
    rt_err_t result;
    for (int i = 0; i < sizeof(soft_spi_config) / sizeof(soft_spi_config[0]); i++)
    {
        soft_spi_bus_obj[i].config = &soft_spi_config[i];
        soft_spi_bus_obj[i].spi_bus.parent.user_data = &soft_spi_config[i];
        result = rt_spi_bus_register(&soft_spi_bus_obj[i].spi_bus, soft_spi_config[i].bus_name, &stm_spi_ops);
        RT_ASSERT(result == RT_EOK);

        rt_kprintf("%s bus init done\n\r", soft_spi_config[i].bus_name);
    }

    return result;
}

rt_err_t rt_soft_spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cspin)
{
    RT_ASSERT(bus_name != RT_NULL);
    RT_ASSERT(device_name != RT_NULL);

    rt_err_t result;
    struct rt_spi_device *spi_device;
    rt_base_t *cs_pin;

    /* initialize the cs pin && select the slave*/
    rt_pin_mode(cspin, PIN_MODE_OUTPUT);
    rt_pin_write(cspin,PIN_HIGH);
    /* attach the device to spi bus*/
    spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);
    cs_pin = (rt_base_t *)rt_malloc(sizeof(rt_base_t));
    RT_ASSERT(cs_pin != RT_NULL);
    *cs_pin = cspin;
    result = rt_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin);

    if (result != RT_EOK)
    {
        rt_kprintf("%s attach to %s faild, %d\n", device_name, bus_name, result);
    }

    RT_ASSERT(result == RT_EOK);

    rt_kprintf("%s attach to %s done \n\r", device_name, bus_name);

    return result;
}



int rt_soft_spi_init(void)
{
    rt_err_t result;
    rt_soft_spi_bus_init();
    result = rt_soft_spi_device_attach("softspi", "spisoft10", RT_USING_SOFT_SPI_CS0_PIN);
    if (result != RT_EOK)
    {
        rt_kprintf("rt_soft_spi_init fail \n\r");
    }

}
INIT_ENV_EXPORT(rt_soft_spi_init);
#endif