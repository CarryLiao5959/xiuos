#include <rtthread.h>
#include <rtdevice.h>
#include <drv_io_config.h>
#include <gpiohs.h>
#include "board.h"
#include "ch438.h"
#include <math.h>
#include <sysctl.h>

/* Offset address of serial port number */
static rt_uint8_t	offsetadd[] = {0x00,0x10,0x20,0x30,0x08,0x18,0x28,0x38,};		
struct rt_serial_device *extuart_serial_parm[8];

void CH438_udelay(rt_uint64_t usec)
{
    if(usec <= 0)
    {
        return;
    }
    rt_uint64_t cycle = read_cycle();
    rt_uint64_t nop_all = usec * sysctl_clock_get_freq(SYSCTL_CLOCK_CPU) / 1000000UL;
    while (1)
    {
        if(read_cycle() - cycle >= nop_all)
            break;
    }
    return;
}

void CH438_INIT(void)
{	
	CH438_set_output();
 	gpiohs_set_drive_mode(FPIOA_CH438_NWR, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_NRD, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_ALE, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_INT, GPIO_DM_INPUT_PULL_UP);
	
	gpiohs_set_pin(FPIOA_CH438_NWR, GPIO_PV_HIGH);
	gpiohs_set_pin(FPIOA_CH438_NRD, GPIO_PV_HIGH);
	gpiohs_set_pin(FPIOA_CH438_ALE, GPIO_PV_HIGH);
}	

void CH438_PORT_INIT( rt_uint8_t ext_uart_no,rt_uint32_t BaudRate )
{
	rt_uint32_t	div;
	rt_uint8_t	DLL,DLM,dlab;
	rt_uint8_t	REG_LCR_ADDR;
	rt_uint8_t	REG_DLL_ADDR;
	rt_uint8_t	REG_DLM_ADDR;
	rt_uint8_t	REG_IER_ADDR;
	rt_uint8_t	REG_MCR_ADDR;
	rt_uint8_t	REG_FCR_ADDR;
	
	REG_LCR_ADDR = offsetadd[ext_uart_no] | REG_LCR0_ADDR;
	REG_DLL_ADDR = offsetadd[ext_uart_no] | REG_DLL0_ADDR;
	REG_DLM_ADDR = offsetadd[ext_uart_no] | REG_DLM0_ADDR;
	REG_IER_ADDR = offsetadd[ext_uart_no] | REG_IER0_ADDR;
	REG_MCR_ADDR = offsetadd[ext_uart_no] | REG_MCR0_ADDR;
	REG_FCR_ADDR = offsetadd[ext_uart_no] | REG_FCR0_ADDR;

	/* Reset the serial port */		
    WriteCH438Data( REG_IER_ADDR, BIT_IER_RESET );             
	rt_thread_mdelay(50);
	
	dlab = ReadCH438Data(REG_IER_ADDR);
	dlab &= 0xDF;
	WriteCH438Data(REG_IER_ADDR, dlab);
	
	/* set LCR register DLAB bit 1 */
	dlab = ReadCH438Data(REG_LCR_ADDR);
	dlab |= 0x80;
	WriteCH438Data(REG_LCR_ADDR, dlab);

    div = ( Fpclk >> 4 ) / BaudRate;
    DLM = div >> 8;
    DLL = div & 0xff;

	/* Set baud rate */
	WriteCH438Data( REG_DLL_ADDR, DLL );             
    WriteCH438Data( REG_DLM_ADDR, DLM );

	/* set FIFO mode, 112 bytes */
	WriteCH438Data( REG_FCR_ADDR, BIT_FCR_RECVTG1 | BIT_FCR_RECVTG0 | BIT_FCR_FIFOEN );
	
	/* 8 bit word size, 1 bit stop bit, no crc */
    WriteCH438Data( REG_LCR_ADDR, BIT_LCR_WORDSZ1 | BIT_LCR_WORDSZ0 );

	/* enable interrupt */
    WriteCH438Data( REG_IER_ADDR, BIT_IER_IERECV );

	/* allow interrupt output, DTR and RTS is 1 */
    WriteCH438Data( REG_MCR_ADDR, BIT_MCR_OUT2 );

	/* release the data in FIFO */
	WriteCH438Data( REG_FCR_ADDR,ReadCH438Data(REG_FCR_ADDR)| BIT_FCR_TFIFORST );
}

static void CH438_set_output(void)
{
 	gpiohs_set_drive_mode(FPIOA_CH438_D0, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_D1, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_D2, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_D3, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_D4, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_D5, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_D6, GPIO_DM_OUTPUT);
	gpiohs_set_drive_mode(FPIOA_CH438_D7, GPIO_DM_OUTPUT);	
}

static void CH438_set_input(void)
{
	gpiohs_set_drive_mode(FPIOA_CH438_D0, GPIO_DM_INPUT_PULL_UP);
	gpiohs_set_drive_mode(FPIOA_CH438_D1, GPIO_DM_INPUT_PULL_UP);
	gpiohs_set_drive_mode(FPIOA_CH438_D2, GPIO_DM_INPUT_PULL_UP);
	gpiohs_set_drive_mode(FPIOA_CH438_D3, GPIO_DM_INPUT_PULL_UP);
	gpiohs_set_drive_mode(FPIOA_CH438_D4, GPIO_DM_INPUT_PULL_UP);
	gpiohs_set_drive_mode(FPIOA_CH438_D5, GPIO_DM_INPUT_PULL_UP);
	gpiohs_set_drive_mode(FPIOA_CH438_D6, GPIO_DM_INPUT_PULL_UP);
	gpiohs_set_drive_mode(FPIOA_CH438_D7, GPIO_DM_INPUT_PULL_UP);
}

rt_uint8_t ReadCH438Data( rt_uint8_t addr )
{
	rt_uint8_t dat = 0;
	gpiohs_set_pin(FPIOA_CH438_NWR,GPIO_PV_HIGH);	
	gpiohs_set_pin(FPIOA_CH438_NRD,GPIO_PV_HIGH);	
	gpiohs_set_pin(FPIOA_CH438_ALE,GPIO_PV_HIGH);	

	CH438_set_output();
	CH438_udelay(1);
	
	if(addr &0x80)	gpiohs_set_pin(FPIOA_CH438_D7,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D7,GPIO_PV_LOW);	
	if(addr &0x40)	gpiohs_set_pin(FPIOA_CH438_D6,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D6,GPIO_PV_LOW);	
	if(addr &0x20)	gpiohs_set_pin(FPIOA_CH438_D5,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D5,GPIO_PV_LOW);	
	if(addr &0x10)	gpiohs_set_pin(FPIOA_CH438_D4,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D4,GPIO_PV_LOW);	
	if(addr &0x08)	gpiohs_set_pin(FPIOA_CH438_D3,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D3,GPIO_PV_LOW);	
	if(addr &0x04)	gpiohs_set_pin(FPIOA_CH438_D2,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D2,GPIO_PV_LOW);	
	if(addr &0x02)	gpiohs_set_pin(FPIOA_CH438_D1,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D1,GPIO_PV_LOW);	
	if(addr &0x01)	gpiohs_set_pin(FPIOA_CH438_D0,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D0,GPIO_PV_LOW);	
		
	CH438_udelay(1);

	gpiohs_set_pin(FPIOA_CH438_ALE,GPIO_PV_LOW);	
	CH438_udelay(1);		

	CH438_set_input();
	CH438_udelay(1);
	
	gpiohs_set_pin(FPIOA_CH438_NRD,GPIO_PV_LOW);	
	CH438_udelay(1);	

	if (gpiohs_get_pin(FPIOA_CH438_D7))	dat |= 0x80;
	if (gpiohs_get_pin(FPIOA_CH438_D6))	dat |= 0x40;
	if (gpiohs_get_pin(FPIOA_CH438_D5))	dat |= 0x20;
	if (gpiohs_get_pin(FPIOA_CH438_D4))	dat |= 0x10;
	if (gpiohs_get_pin(FPIOA_CH438_D3))	dat |= 0x08;
	if (gpiohs_get_pin(FPIOA_CH438_D2))	dat |= 0x04;
	if (gpiohs_get_pin(FPIOA_CH438_D1))	dat |= 0x02;
	if (gpiohs_get_pin(FPIOA_CH438_D0))	dat |= 0x01;
	
	gpiohs_set_pin(FPIOA_CH438_NRD,GPIO_PV_HIGH);	
	gpiohs_set_pin(FPIOA_CH438_ALE,GPIO_PV_HIGH);	
	CH438_udelay(1);

	return dat;
}
	
static void WriteCH438Data( rt_uint8_t addr, rt_uint8_t dat )
{
	gpiohs_set_pin(FPIOA_CH438_ALE,GPIO_PV_HIGH);	
	gpiohs_set_pin(FPIOA_CH438_NRD,GPIO_PV_HIGH);	
	gpiohs_set_pin(FPIOA_CH438_NWR,GPIO_PV_HIGH);	

	CH438_set_output();
	CH438_udelay(1);	
	
	if(addr &0x80)	gpiohs_set_pin(FPIOA_CH438_D7,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D7,GPIO_PV_LOW);	
	if(addr &0x40)	gpiohs_set_pin(FPIOA_CH438_D6,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D6,GPIO_PV_LOW);	
	if(addr &0x20)	gpiohs_set_pin(FPIOA_CH438_D5,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D5,GPIO_PV_LOW);	
	if(addr &0x10)	gpiohs_set_pin(FPIOA_CH438_D4,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D4,GPIO_PV_LOW);	
	if(addr &0x08)	gpiohs_set_pin(FPIOA_CH438_D3,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D3,GPIO_PV_LOW);	
	if(addr &0x04)	gpiohs_set_pin(FPIOA_CH438_D2,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D2,GPIO_PV_LOW);	
	if(addr &0x02)	gpiohs_set_pin(FPIOA_CH438_D1,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D1,GPIO_PV_LOW);	
	if(addr &0x01)	gpiohs_set_pin(FPIOA_CH438_D0,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D0,GPIO_PV_LOW);	
	
	CH438_udelay(1);	
	
	gpiohs_set_pin(FPIOA_CH438_ALE,GPIO_PV_LOW);	
	CH438_udelay(1);
	
	if(dat &0x80)	gpiohs_set_pin(FPIOA_CH438_D7,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D7,GPIO_PV_LOW);	
	if(dat &0x40)	gpiohs_set_pin(FPIOA_CH438_D6,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D6,GPIO_PV_LOW);	
	if(dat &0x20)	gpiohs_set_pin(FPIOA_CH438_D5,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D5,GPIO_PV_LOW);	
	if(dat &0x10)	gpiohs_set_pin(FPIOA_CH438_D4,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D4,GPIO_PV_LOW);	
	if(dat &0x08)	gpiohs_set_pin(FPIOA_CH438_D3,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D3,GPIO_PV_LOW);	
	if(dat &0x04)	gpiohs_set_pin(FPIOA_CH438_D2,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D2,GPIO_PV_LOW);	
	if(dat &0x02)	gpiohs_set_pin(FPIOA_CH438_D1,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D1,GPIO_PV_LOW);	
	if(dat &0x01)	gpiohs_set_pin(FPIOA_CH438_D0,GPIO_PV_HIGH);	else	gpiohs_set_pin(FPIOA_CH438_D0,GPIO_PV_LOW);	
	
	CH438_udelay(1);	

	gpiohs_set_pin(FPIOA_CH438_NWR,GPIO_PV_LOW);	
	CH438_udelay(1);	
	
	gpiohs_set_pin(FPIOA_CH438_NWR,GPIO_PV_HIGH);	
	gpiohs_set_pin(FPIOA_CH438_ALE,GPIO_PV_HIGH);	
	CH438_udelay(1);	

	CH438_set_input();

	return;
}

static void WriteCH438Block( rt_uint8_t mAddr, rt_uint8_t mLen, rt_uint8_t *mBuf )   
{
    while ( mLen -- ) 	
	  WriteCH438Data( mAddr, *mBuf++ );
}

static int CH438Irq(void *parameter)
{
	rt_uint8_t gInterruptStatus;
	rt_uint8_t port = 0;
	struct rt_serial_device *serial = (struct rt_serial_device *)parameter;
	/* multi irq may happen*/
	gInterruptStatus = ReadCH438Data(REG_SSR_ADDR);
	port = log(gInterruptStatus & 0xFF)/log(2);

	rt_hw_serial_isr(extuart_serial_parm[port], RT_SERIAL_EVENT_RX_IND);
}

static rt_err_t rt_extuart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
	rt_uint32_t baud_rate = cfg->baud_rate;
	rt_uint16_t port = cfg->reserved;

	CH438_PORT_INIT(port, baud_rate);
	return RT_EOK;
}

static rt_err_t extuart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
	rt_uint16_t ext_uart_no = serial->config.reserved;
	static rt_uint16_t register_flag = 0;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
		if(1 == register_flag)
		{
			gpiohs_irq_unregister(FPIOA_CH438_INT);
			register_flag = 0;
		}
        break;
    case RT_DEVICE_CTRL_SET_INT:
		if(0 == register_flag)
		{
			gpiohs_set_drive_mode(FPIOA_CH438_INT, GPIO_DM_INPUT_PULL_UP);
    		gpiohs_set_pin_edge(FPIOA_CH438_INT,GPIO_PE_FALLING);
			gpiohs_irq_register(FPIOA_CH438_INT, 1, CH438Irq, (void*)serial);
			register_flag = 1;
		}
		break;
    }
    return (RT_EOK);
}

static int drv_extuart_putc(struct rt_serial_device *serial, char c)
{
	rt_uint16_t ext_uart_no = serial->config.reserved;
	rt_uint8_t	REG_LSR_ADDR,REG_THR_ADDR;
	
	REG_LSR_ADDR = offsetadd[ext_uart_no] | REG_LSR0_ADDR;
	REG_THR_ADDR = offsetadd[ext_uart_no] | REG_THR0_ADDR;
	
	rt_thread_mdelay(1);	
	if((ReadCH438Data( REG_LSR_ADDR ) & BIT_LSR_TEMT) != 0)
	{
		WriteCH438Block( REG_THR_ADDR, 1, &c );
		return 1;
	} else {
		return 0;
	}
}

static int drv_extuart_getc(struct rt_serial_device *serial)
{
	rt_uint8_t	dat = 0;
	rt_uint8_t	REG_LSR_ADDR,REG_RBR_ADDR;
	rt_uint16_t ext_uart_no = serial->config.reserved;///< get extern uart port
	
	REG_LSR_ADDR = offsetadd[ext_uart_no] | REG_LSR0_ADDR;
	REG_RBR_ADDR = offsetadd[ext_uart_no] | REG_RBR0_ADDR;

	rt_thread_mdelay(1);
	if((ReadCH438Data(REG_LSR_ADDR) & BIT_LSR_DATARDY) == 0x01)
	{
		dat = ReadCH438Data( REG_RBR_ADDR );
		if(dat >= 0)
			return dat;
	} else {
		return -1;
	}
}

const struct rt_uart_ops extuart_ops =
{
    rt_extuart_configure,
    extuart_control,
    drv_extuart_putc,
    drv_extuart_getc,
    RT_NULL
};

int rt_hw_ch438_init(void)
{
    struct rt_serial_device *extserial;
    struct device_uart      *extuart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
	rt_err_t ret;
	#ifdef CONFIG_CH438_EXTUART0
    {// extserial0
        static struct rt_serial_device  extserial0;

        extserial  = &extserial0;
        extserial->ops              = &extuart_ops;
        extserial->config           = config;
        extserial->config.baud_rate = 115200;
		extserial->config.reserved  = 0; ///< extern uart port
		extuart_serial_parm[0] = &extserial0;

        ret = rt_hw_serial_register(extserial,
                              "extuart_dev0",
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              extuart);
		if(ret < 0){
			rt_kprintf("extuart_dev0 register failed.\n");
		}
    }
	#endif
	#ifdef CONFIG_CH438_EXTUART1
	{// extserial1
        static struct rt_serial_device  extserial1;

        extserial  = &extserial1;
        extserial->ops              = &extuart_ops;
        extserial->config           = config;
        extserial->config.baud_rate = 115200;
		extserial->config.reserved = 1; ///< extern uart port
        extuart_serial_parm[1] = &extserial1;

        ret = rt_hw_serial_register(extserial,
                              "extuart_dev1",
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              extuart);
		if(ret < 0){
			rt_kprintf("extuart_dev1 register failed.\n");
		}
    }
	#endif
	#ifdef CONFIG_CH438_EXTUART2
	{// extserial2
        static struct rt_serial_device  extserial2;

        extserial  = &extserial2;
        extserial->ops              = &extuart_ops;
        extserial->config           = config;
        extserial->config.baud_rate = 9600;
		extserial->config.reserved = 2; ///< extern uart port
        extuart_serial_parm[2] = &extserial2;

       	ret = rt_hw_serial_register(extserial,
                              "extuart_dev2",
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              extuart);
		if(ret < 0){
			rt_kprintf("extuart_dev2 register failed.\n");
		}
    }
	#endif
	#ifdef CONFIG_CH438_EXTUART3
	{// extserial3
        static struct rt_serial_device  extserial3;

        extserial  = &extserial3;
        extserial->ops              = &extuart_ops;
        extserial->config           = config;
        extserial->config.baud_rate = 9600;
		extserial->config.reserved = 3; ///< extern uart port
		extuart_serial_parm[3] = &extserial3;

       	ret = rt_hw_serial_register(extserial,
                              "extuart_dev3",
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              extuart);
		if(ret < 0){
			rt_kprintf("extuart_dev3 register failed.\n");
		}
    }
	#endif
	#ifdef CONFIG_CH438_EXTUART4
	{// extserial4
        static struct rt_serial_device  extserial4;

        extserial  = &extserial4;
        extserial->ops              = &extuart_ops;
        extserial->config           = config;
        extserial->config.baud_rate = 115200;
		extserial->config.reserved = 4; ///< extern uart port
		extuart_serial_parm[4] = &extserial4;

       	ret = rt_hw_serial_register(extserial,
                              "extuart_dev4",
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              extuart);
		if(ret < 0){
			rt_kprintf("extuart_dev4 register failed.\n");
		}
    }
	#endif
	#ifdef CONFIG_CH438_EXTUART5
	{// extserial5
        static struct rt_serial_device  extserial5;

        extserial  = &extserial5;
        extserial->ops              = &extuart_ops;
        extserial->config           = config;
        extserial->config.baud_rate = 115200;
		extserial->config.reserved = 5; ///< extern uart port
		extuart_serial_parm[5] = &extserial5;

       	ret = rt_hw_serial_register(extserial,
                              "extuart_dev5",
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              extuart);
		if(ret < 0){
			rt_kprintf("extuart_dev5 register failed.\n");
		}
    }
	#endif
	#ifdef CONFIG_CH438_EXTUART6
	{// extserial6
        static struct rt_serial_device  extserial6;

        extserial  = &extserial6;
        extserial->ops              = &extuart_ops;
        extserial->config           = config;
        extserial->config.baud_rate = 115200;
		extserial->config.reserved = 6; ///< extern uart port
		extuart_serial_parm[6] = &extserial6;

       	ret = rt_hw_serial_register(extserial,
                              "extuart_dev6",
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              extuart);
		if(ret < 0){
			rt_kprintf("extuart_dev6 register failed.\n");
		}
    }
	#endif
	#ifdef CONFIG_CH438_EXTUART7
	{// extserial7
        static struct rt_serial_device  extserial7;

        extserial  = &extserial7;
        extserial->ops              = &extuart_ops;
        extserial->config           = config;
        extserial->config.baud_rate = 115200;
		extserial->config.reserved = 7; ///< extern uart port
		extuart_serial_parm[7] = &extserial7;

       	ret = rt_hw_serial_register(extserial,
                              "extuart_dev7",
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              extuart);
		if(ret < 0){
			rt_kprintf("extuart_dev7 register failed.\n");
		}
    }
	#endif
	CH438_INIT();

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_ch438_init);

/* Test CH438 register */
void CH438_RegTEST(rt_uint8_t num)
{
	rt_uint8_t dat;

	rt_kprintf("current test serilnum:  %02x \r\n",offsetadd[num]);
	rt_kprintf("IER: %02x\r\n",ReadCH438Data(offsetadd[num] | REG_IER0_ADDR));//?IER
	rt_kprintf("IIR: %02x\r\n",ReadCH438Data(offsetadd[num] | REG_IIR0_ADDR));//?IIR
	rt_kprintf("LCR: %02x\r\n",ReadCH438Data(offsetadd[num] | REG_LCR0_ADDR));//?LCR
	rt_kprintf("MCR: %02x\r\n",ReadCH438Data(offsetadd[num] | REG_MCR0_ADDR));//?MCR
	rt_kprintf("LSR: %02x\r\n",ReadCH438Data(offsetadd[num] | REG_LSR0_ADDR));//?LSR
	rt_kprintf("MSR: %02x\r\n",ReadCH438Data(offsetadd[num] | REG_MSR0_ADDR));//?MSR
	rt_kprintf("FCR: %02x\r\n",ReadCH438Data(offsetadd[num] | REG_FCR0_ADDR));//?FCR
	rt_kprintf("SSR: %02x\r\n",ReadCH438Data( offsetadd[num] | REG_SSR_ADDR ));//?SSR
	
	rt_kprintf("SCR0: %02x\r\n",(rt_uint8_t)ReadCH438Data(offsetadd[num] | REG_SCR0_ADDR));//?SCR
	dat = 0x55;
	WriteCH438Data(offsetadd[num] | REG_SCR0_ADDR, dat);
	rt_kprintf("SCR55: %02x\r\n",(rt_uint8_t)ReadCH438Data(offsetadd[num] | REG_SCR0_ADDR));//?SCR
	dat = 0xAA;
	WriteCH438Data(offsetadd[num] | REG_SCR0_ADDR, dat);
	rt_kprintf("SCRAA: %02x\r\n",(rt_uint8_t)ReadCH438Data(offsetadd[num] | REG_SCR0_ADDR));//?SCR
}

void CH438_RegsTEST(rt_uint8_t num)
{
	CH438_RegTEST(0);
	CH438_RegTEST(1);
	CH438_RegTEST(2);
	CH438_RegTEST(3);
	CH438_RegTEST(4);
	CH438_RegTEST(5);
	CH438_RegTEST(6);
	CH438_RegTEST(7);
}
MSH_CMD_EXPORT(CH438_RegsTEST, CH438_RegsTEST all);