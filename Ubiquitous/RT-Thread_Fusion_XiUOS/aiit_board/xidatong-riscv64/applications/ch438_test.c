#include <rtthread.h>
#include <rtdevice.h>
#include "ch438.h"

#define CH438_DEVICE_NAME    "ch438"
static struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  
static rt_device_t serial7;
static rt_device_t serial2;

/****************************************************************************
 * Name: CH438Test
 *
 * Description:
 *   Test ch438 port 7
 *   use port 7 ,send data to buf
 ****************************************************************************/
int CH438_TEST(void) {
    rt_uint8_t buff[64]={0};
	rt_uint8_t str[]="rt device write successfully!123+###\r\n";
	rt_size_t size,i; 
	rt_err_t res = RT_EOK;

	/* Find extended serial port */
	serial7=rt_device_find("extuart_dev7");    
	if (!serial7) {
        rt_kprintf("find %s failed!\n", CH438_DEVICE_NAME);
        return RT_ERROR;
    }
	
	/* Open extended serial port 7 (Including initializing the serial port) */
	res=rt_device_open(serial7, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);	
	if (res != RT_EOK) {
        rt_kprintf("open %s failed!\n", CH438_DEVICE_NAME);
        return res;
    } 

	/* Test Sending and Receiving Data */
	while(1) {
		rt_device_write(serial7, 0 , str, sizeof(str));
		size=rt_device_read(serial7,0, buff, (sizeof(buff)));
		rt_kprintf("receving size = %d\n",size);
		for(i=0; i<size; ++i)
		{
			rt_kprintf("ch=%c\n",buff[i]);
		}
		rt_thread_mdelay(1000);
	}
}	
MSH_CMD_EXPORT(CH438_TEST,ch438 test sample);

/****************************************************************************
 * Name: HC08Test
 *
 * Description:
 *   Test hc08 
 *   use port 2 
 ****************************************************************************/
void HC08Test(void){
	rt_uint8_t buff2[128]={0};
	rt_uint8_t time=10;
	
	rt_err_t res = RT_EOK;

  	serial2=rt_device_find("extuart_dev2");
	if (!serial2) {
		rt_kprintf("find %s failed!\n", "hc08");
		return RT_ERROR;
    }

	res=rt_device_open(serial2, RT_DEVICE_OFLAG_RDWR);
	if (res != RT_EOK) {
		rt_kprintf("open %s failed!\n", "hc08");
		return res;
    } 

	rt_device_write(serial2, 0, "AT",sizeof("AT"));
	rt_thread_mdelay(1000);
	rt_device_write(serial2, 0, "AT+DEFAULT",sizeof("AT+DEFAULT"));
   	rt_thread_mdelay(1000);
	
   	while(time--) {
		rt_kprintf("waiting data...\r\n");
		while(rt_device_read(serial2,0, buff2, sizeof(buff2))==0);
		rt_kprintf("buff2:%s\r\n",buff2);
		rt_thread_mdelay(2000);
		rt_memset(buff2, 0, 128);
	 } 
}
MSH_CMD_EXPORT(HC08Test,hc08 test sample);
