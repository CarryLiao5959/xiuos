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

/**
* @file ch376.c
* @brief xidatong-riscv64-board ch376.c
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2022-10-26
*/

#include <rtthread.h>
#include <rtdevice.h>
#include "ch376.h"
#include "ch376_fs.h"

static rt_device_t uart3_serial;
static struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  

void CH376_PORT_INIT( void )  
{
	rt_err_t ret = RT_EOK;

	/* Find the serial port devices*/
    uart3_serial = rt_device_find("uart3");
    if (!uart3_serial)
    {
        rt_kprintf("Find %s failed!\n", "uart3");
    }

    /* Modify the serial port configuration parameters */
    config.baud_rate = BAUD_RATE_115200;        
    config.data_bits = DATA_BITS_8;           
    config.stop_bits = STOP_BITS_1;           
    config.bufsz     = 128;             // Change the buff size to 128
    config.parity    = PARITY_NONE;    
    rt_device_control(uart3_serial, RT_DEVICE_CTRL_CONFIG, &config);

    /* Turn on the serial port device in interrupt receive and poll send mode */
    ret = rt_device_open(uart3_serial, RT_DEVICE_FLAG_INT_RX);
    if (ret != RT_EOK)
    {
        rt_kprintf("Open %s device failed!\n", "uart3");
    }
}

void xWriteCH376Cmd( rt_uint8_t mCmd )  /* Write the command to CH376 */
{
	rt_uint8_t write_cmd[3];
	write_cmd[0] = SER_SYNC_CODE1;
	write_cmd[1] = SER_SYNC_CODE2;
	write_cmd[2] = mCmd;
	rt_thread_mdelay(5);
	rt_device_write(uart3_serial, 0, write_cmd, 3);
}

void xEndCH376Cmd(void) /* End the CH376 command, which is used only on the SPI interface */
{
}

void xWriteCH376Data( rt_uint8_t mData )  /* Write data to CH376 */
{
	rt_uint8_t tmp = mData;
	rt_device_write(uart3_serial, 0, &tmp, 1);
	rt_thread_mdelay(2);
}

rt_uint8_t xReadCH376Data( void )  /* Read data from the CH376 */
{
	rt_uint32_t i;
    rt_uint8_t data;
    int  res;
    for(i=0;i<500000;i++)
    {
        res = rt_device_read(uart3_serial, 0, &data, 1);
        if(res == 1)
        {
			return ((rt_uint8_t)data);
        }
        rt_thread_mdelay(1);
    }
    return ERR_USB_UNKNOWN;
}

rt_uint8_t	Query376Interrupt( void ) /* Query CH376 interrupt */
{
    //When an interrupt occurs, the serial port will receive a data, read it directly and discard it
    if(xReadCH376Data() == ERR_USB_UNKNOWN) return FALSE ;
    else return TRUE ;
}

rt_uint8_t	mInitCH376Host( void ) /* CH376 INIT */
{
	rt_uint8_t	res;
	/* After power on, delay operation for at least 50ms */
	rt_thread_mdelay(50);

	/* CH376 port init */
	CH376_PORT_INIT( );
	rt_thread_mdelay(600);

	/* Test the communication interface between SCM and CH376 */
	xWriteCH376Cmd( CMD11_CHECK_EXIST );
	xWriteCH376Data( 0x65 );  // CH376会按位取反返回数据,发送0x65,则返回0x9A,代表通讯成功
	res = xReadCH376Data( );
	if ( res != 0x9A )
	{
		rt_kprintf("CH376 module connection failed\n");
		return( ERR_USB_UNKNOWN );  
	}
	else
	{
		rt_kprintf("CH376 module connection successful!\n");
	}

	/* SET USB MODE */
	xWriteCH376Cmd( CMD11_SET_USB_MODE );
	xWriteCH376Data( CH376_WORK_MODE );
	rt_thread_mdelay(20);
	res = xReadCH376Data( );
	if ( res == CMD_RET_SUCCESS )
	{
		rt_kprintf("Set USB mode successful!\n");
		return(USB_INT_SUCCESS);
	}
	else
	{
		rt_kprintf("Set USB mode error.\n");
		return( ERR_USB_UNKNOWN );  
	}
}
