#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#endif

#define WRITE_SIZE (1024)
#define ONCE_SIZE (256)
#define BIG_BLOCK (1024*1024*2)
void sd_write_speed_test()
{
    printf("sd_write_speed_test \n");
    int fd = 0;
    int ret = 0;
    rt_uint32_t tick1  = rt_tick_get();
    char ready_writedata[ONCE_SIZE] ={0};
    memset(ready_writedata,0xAE,ONCE_SIZE);
    
    char *ready_writedata1 = NULL;
    
    ready_writedata1 = rt_malloc(BIG_BLOCK);
    memset(ready_writedata1,0xBE,BIG_BLOCK);
    
    printf("the moment tick is %d\n",tick1);
    fd = open("/ready_writedata", O_WRONLY | O_CREAT);
    if(fd >0)
    {
        printf("fd >0 \n");
    }
    else
    {
         printf("fd <0 \n");
         return;
    }
    
      for(int i = 0;i <1;i++)
    {
  
          ret = write(fd,ready_writedata1,BIG_BLOCK);
    }
    rt_uint32_t tick2  = rt_tick_get();
    printf("ret is %d \n",ret);
    printf("the moment tick is %d\n",tick2);
    printf(" time-consuming  %d s\n",(tick2 - tick1)/1000);
    ret = close(fd);
    printf("ret is %d \n",ret);
    free(ready_writedata1);
    return;

}
MSH_CMD_EXPORT(sd_write_speed_test,write_speed);






