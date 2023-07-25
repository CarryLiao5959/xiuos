#include <rtthread.h>
#include <rtdevice.h>

#define RS232    "RS232"
static struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  
static rt_device_t serial2;

/****************************************************************************
 * Name: RS232Test
 *
 * Description:
 *   Test RS232
 ****************************************************************************/
int RS232_test(int argc, char *argv[]) {
    rt_uint8_t buff[64]={0};
	char str[]="RS232 test!!!\r\n";
	rt_size_t size,i; 
	rt_err_t res = RT_EOK;

    char uart_name[RT_NAME_MAX];


    if (argc == 2)
    {
        rt_strncpy(uart_name, argv[1], RT_NAME_MAX);
    }
    else
    {
        rt_strncpy(uart_name, "uart6", RT_NAME_MAX);
    }

	/* Find extended serial port */
	serial2=rt_device_find(uart_name);    
	if (!serial2) {
        rt_kprintf("find %s failed!\n", RS232);
        return RT_ERROR;
    }
	
	/* Open extended serial port 7 (Including initializing the serial port) */
	res=rt_device_open(serial2, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);	
	if (res != RT_EOK) {
        rt_kprintf("open %s failed!\n", RS232);
        return res;
    } 
	
	/* Test Sending and Receiving Data */
	int cnt=0;
	while(cnt<10) {
		rt_size_t writeData = rt_device_write(serial2, 0, str, (sizeof(str) - 1));
		size=rt_device_read(serial2, 0, buff, (sizeof(buff)));
		rt_kprintf("receving size = %d\n",size);
		cnt++;
		for(i=0; i<size; ++i)
		{
			rt_kprintf("ch=%c\n",buff[i]);
		}
		rt_thread_mdelay(1000);
	}
}	
MSH_CMD_EXPORT(RS232_test, RS232 test sample);