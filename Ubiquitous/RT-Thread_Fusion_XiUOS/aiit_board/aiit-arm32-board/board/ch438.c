#include <rtthread.h>
#include <rtdevice.h>
#include "ch438.h"
#include <math.h>

/* Offset address of serial port number */
static rt_uint8_t	offsetadd[] = {0x00,0x10,0x20,0x30,0x08,0x18,0x28,0x38,};		
struct rt_serial_device *extuart_serial_parm[8];

static void CH438_udelay(rt_uint64_t us)
{ 
    rt_uint64_t ticks;
    rt_uint64_t told, tnow, tcnt = 0;
    rt_uint64_t reload = SysTick->LOAD;

    ticks = us * reload / (1000000 / RT_TICK_PER_SECOND);
    told = SysTick->VAL;
    while (1) {
        tnow = SysTick->VAL;
        if (tnow != told) {
            if (tnow < told) {
                tcnt += told - tnow;
            }
            else {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) {
                break;
            }
        }
    }
}

void CH438_INIT(void)
{	
    CH438_set_output();
    rt_pin_mode(CH438_NWR_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_NRD_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_NCS_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_ALE_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DIR_485CH1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DIR_485CH2_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DIR_485CH3_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_INT_PIN, PIN_MODE_INPUT_PULLUP);

    rt_pin_write(CH438_NWR_PIN, PIN_HIGH);
    rt_pin_write(CH438_NRD_PIN, PIN_HIGH);
    rt_pin_write(CH438_NCS_PIN, PIN_HIGH);
    rt_pin_write(CH438_ALE_PIN, PIN_HIGH);
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
    rt_pin_mode(CH438_D0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_D1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_D2_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_D3_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_D4_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_D5_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_D6_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(CH438_D7_PIN, PIN_MODE_OUTPUT);  
}

static void CH438_set_input(void)
{
    rt_pin_mode(CH438_D0_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(CH438_D1_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(CH438_D2_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(CH438_D3_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(CH438_D4_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(CH438_D5_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(CH438_D6_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(CH438_D7_PIN, PIN_MODE_INPUT_PULLUP);
}

void set_485_input(rt_uint8_t ch_no)
{
	if(ch_no == 1) {
        rt_pin_write(DIR_485CH1_PIN, PIN_LOW);
    }

    if(ch_no == 2) {
        rt_pin_write(DIR_485CH2_PIN, PIN_LOW);
    }

    if(ch_no == 3) {
        rt_pin_write(DIR_485CH3_PIN, PIN_LOW);
    }		
}

void set_485_output(rt_uint8_t ch_no)
{
	if(ch_no == 1) {
        rt_pin_write(DIR_485CH1_PIN, PIN_HIGH);
    }

    if(ch_no == 2) {
        rt_pin_write(DIR_485CH2_PIN, PIN_HIGH);
    }

    if(ch_no == 3) {
        rt_pin_write(DIR_485CH3_PIN, PIN_HIGH);
    }
}

rt_uint8_t ReadCH438Data( rt_uint8_t addr )
{
    rt_uint8_t dat = 0;
    rt_pin_write(CH438_NWR_PIN, PIN_HIGH);
    rt_pin_write(CH438_NRD_PIN, PIN_HIGH);
    rt_pin_write(CH438_ALE_PIN, PIN_HIGH);
    rt_pin_write(CH438_NCS_PIN, PIN_HIGH);

    CH438_set_output();
    CH438_udelay(1);

    if(addr &0x80)	rt_pin_write(CH438_D7_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D7_PIN,PIN_LOW);	
    if(addr &0x40)	rt_pin_write(CH438_D6_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D6_PIN,PIN_LOW);	
    if(addr &0x20)	rt_pin_write(CH438_D5_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D5_PIN,PIN_LOW);	
    if(addr &0x10)	rt_pin_write(CH438_D4_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D4_PIN,PIN_LOW);	
    if(addr &0x08)	rt_pin_write(CH438_D3_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D3_PIN,PIN_LOW);	
    if(addr &0x04)	rt_pin_write(CH438_D2_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D2_PIN,PIN_LOW);	
    if(addr &0x02)	rt_pin_write(CH438_D1_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D1_PIN,PIN_LOW);	
    if(addr &0x01)	rt_pin_write(CH438_D0_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D0_PIN,PIN_LOW);	
      
    CH438_udelay(1);

    rt_pin_write(CH438_ALE_PIN,PIN_LOW);	
    CH438_udelay(1);		
    rt_pin_write(CH438_NCS_PIN, PIN_LOW);
    CH438_udelay(1);	

    CH438_set_input();
    CH438_udelay(1);

    rt_pin_write(CH438_NRD_PIN,PIN_LOW);	
    CH438_udelay(1);	

    if (rt_pin_read(CH438_D7_PIN))	dat |= 0x80;
    if (rt_pin_read(CH438_D6_PIN))	dat |= 0x40;
    if (rt_pin_read(CH438_D5_PIN))	dat |= 0x20;
    if (rt_pin_read(CH438_D4_PIN))	dat |= 0x10;
    if (rt_pin_read(CH438_D3_PIN))	dat |= 0x08;
    if (rt_pin_read(CH438_D2_PIN))	dat |= 0x04;
    if (rt_pin_read(CH438_D1_PIN))	dat |= 0x02;
    if (rt_pin_read(CH438_D0_PIN))	dat |= 0x01;

    rt_pin_write(CH438_NRD_PIN,PIN_HIGH);	
    rt_pin_write(CH438_ALE_PIN,PIN_HIGH);	
    rt_pin_write(CH438_NCS_PIN,PIN_HIGH);
    CH438_udelay(1);

    return dat;
}

static void WriteCH438Data( rt_uint8_t addr, char dat )
{
    rt_pin_write(CH438_ALE_PIN, PIN_HIGH);
    rt_pin_write(CH438_NWR_PIN, PIN_HIGH);
    rt_pin_write(CH438_NRD_PIN, PIN_HIGH);
    rt_pin_write(CH438_NCS_PIN, PIN_HIGH);

    CH438_set_output();
    CH438_udelay(1);	

    if(addr &0x80)	rt_pin_write(CH438_D7_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D7_PIN,PIN_LOW);	
    if(addr &0x40)	rt_pin_write(CH438_D6_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D6_PIN,PIN_LOW);	
    if(addr &0x20)	rt_pin_write(CH438_D5_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D5_PIN,PIN_LOW);	
    if(addr &0x10)	rt_pin_write(CH438_D4_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D4_PIN,PIN_LOW);	
    if(addr &0x08)	rt_pin_write(CH438_D3_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D3_PIN,PIN_LOW);	
    if(addr &0x04)	rt_pin_write(CH438_D2_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D2_PIN,PIN_LOW);	
    if(addr &0x02)	rt_pin_write(CH438_D1_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D1_PIN,PIN_LOW);	
    if(addr &0x01)	rt_pin_write(CH438_D0_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D0_PIN,PIN_LOW);	

    CH438_udelay(1);	

    rt_pin_write(CH438_ALE_PIN,PIN_LOW);	
    CH438_udelay(1);
    rt_pin_write(CH438_NCS_PIN, PIN_LOW);
    CH438_udelay(1);

    if(dat &0x80)	rt_pin_write(CH438_D7_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D7_PIN,PIN_LOW);	
    if(dat &0x40)	rt_pin_write(CH438_D6_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D6_PIN,PIN_LOW);	
    if(dat &0x20)	rt_pin_write(CH438_D5_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D5_PIN,PIN_LOW);	
    if(dat &0x10)	rt_pin_write(CH438_D4_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D4_PIN,PIN_LOW);	
    if(dat &0x08)	rt_pin_write(CH438_D3_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D3_PIN,PIN_LOW);	
    if(dat &0x04)	rt_pin_write(CH438_D2_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D2_PIN,PIN_LOW);	
    if(dat &0x02)	rt_pin_write(CH438_D1_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D1_PIN,PIN_LOW);	
    if(dat &0x01)	rt_pin_write(CH438_D0_PIN,PIN_HIGH);	else	rt_pin_write(CH438_D0_PIN,PIN_LOW);	

    CH438_udelay(1);	

    rt_pin_write(CH438_NWR_PIN,PIN_LOW);	
    CH438_udelay(1);	

    rt_pin_write(CH438_NWR_PIN,PIN_HIGH);	
    rt_pin_write(CH438_ALE_PIN,PIN_HIGH);	
    rt_pin_write(CH438_NCS_PIN,PIN_HIGH);
    CH438_udelay(1);	

    CH438_set_input();

    return;
}

static void WriteCH438Block( rt_uint8_t mAddr, rt_uint8_t mLen, char *mBuf )   
{
    while ( mLen -- ) {
        WriteCH438Data( mAddr, *mBuf++ );
    }	  
}

static void CH438Irq(void *parameter)
{
    rt_uint8_t gInterruptStatus;
    rt_uint8_t port = 0;
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
    static rt_uint16_t register_flag = 0;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        if(1 == register_flag)
        {
            rt_pin_irq_enable(CH438_INT_PIN, PIN_IRQ_DISABLE);
            register_flag = 0;
        }
        break;
    case RT_DEVICE_CTRL_SET_INT:
        if(0 == register_flag)
        { 
            rt_pin_mode(CH438_INT_PIN, PIN_MODE_INPUT_PULLUP);
            rt_pin_attach_irq(CH438_INT_PIN, PIN_IRQ_MODE_FALLING, CH438Irq, (void *)serial);
            rt_pin_irq_enable(CH438_INT_PIN, PIN_IRQ_ENABLE);
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
    
    if (2 == ext_uart_no) {
		set_485_output(2);
		CH438_udelay(950);
	} else if (3 == ext_uart_no) {
		set_485_output(1);
		CH438_udelay(950);
	} else if (7 == ext_uart_no) {
		set_485_output(3);
		CH438_udelay(950);
	} 
    #if defined APPLICATION_SENSOR_WINDSPEED_QS_FS || defined APPLICATION_SENSOR_WINDDIRECTION_QS_FX
    #else
    rt_thread_mdelay(2);
    #endif	
    if((ReadCH438Data( REG_LSR_ADDR ) & BIT_LSR_TEMT) != 0)
    {
        WriteCH438Block( REG_THR_ADDR, 1, &c );
        if (2 == ext_uart_no) {
            CH438_udelay(950);
            set_485_input(2);
        } else if (3 == ext_uart_no) {
            CH438_udelay(950);
            set_485_input(1);
        } else if (7 == ext_uart_no) {
            CH438_udelay(950);
            set_485_input(3);
        }
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

    rt_thread_mdelay(2);
    if((ReadCH438Data(REG_LSR_ADDR) & BIT_LSR_DATARDY) == 0x01)
    {
      dat = ReadCH438Data( REG_RBR_ADDR );
      if(dat >= 0) return dat;
      else return -1;
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
	{// extserial7
        static struct rt_serial_device  extserial7;

        extserial  = &extserial7;
        extserial->ops              = &extuart_ops;
        extserial->config           = config;
        extserial->config.baud_rate = 9600;
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
	rt_kprintf("SSR: %02x\r\n",ReadCH438Data(offsetadd[num] | REG_SSR_ADDR ));//?SSR
	
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


/* Test CH438 extended uart */
#define CH438_DEVICE_NAME    "ch438"
static rt_device_t serial6;

int CH438_TEST_UART6(void) {
    rt_uint8_t buff[64]={0};
	rt_uint8_t str[]="rt device write successfully!123+###\r\n";
	rt_size_t size,i; 
	rt_err_t res = RT_EOK;

	serial6=rt_device_find("extuart_dev6");    
	if (!serial6) {
        rt_kprintf("find %s failed!\n", CH438_DEVICE_NAME);
        return RT_ERROR;
    }
	
	res=rt_device_open(serial6, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);	
	if (res != RT_EOK) {
        rt_kprintf("open %s failed!\n", CH438_DEVICE_NAME);
        return res;
    } 

	/* Test Sending and Receiving Data */
	while(1) {
		rt_device_write(serial6, 0 , str, sizeof(str));
		size=rt_device_read(serial6, 0, buff, (sizeof(buff)));
		rt_kprintf("receving size = %d\n",size);
		for(i=0; i<size; ++i) {
			rt_kprintf("ch=%c\n",buff[i]);
		}
		rt_thread_mdelay(1000);
        rt_memset(buff, 0, 64);
	}
}	
MSH_CMD_EXPORT(CH438_TEST_UART6,ch438 test sample);
