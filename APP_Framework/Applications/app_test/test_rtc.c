#include <stdio.h>
#include <string.h>
#include <transform.h>

void TestRTC(int argc,char *argv[])
{
    int rtc_fd = PrivOpen(RTC_DEV_DRIVER, O_RDWR);
    if(rtc_fd<0){
        printf("open rtc fd error:%d\n",rtc_fd);
        return;
    }
    
    if(argc>1){
        
        int times = atoi(argv[1]);
        printf("Time will be printf %d times\n",times);
        struct RtcDrvConfigureParam rtc_para;
        rtc_para.rtc_operation_cmd = OPER_RTC_SET_TIME;
        *(rtc_para.time) = 0;

        struct PrivIoctlCfg ioctl_cfg;
        ioctl_cfg.ioctl_driver_type = RTC_TYPE;
        ioctl_cfg.args =  (void *)&rtc_para;
        PrivIoctl(rtc_fd,0,&ioctl_cfg);

        rtc_para.rtc_operation_cmd = OPER_RTC_GET_TIME;
        for(size_t i=0;i<times;i++){
            PrivIoctl(rtc_fd,0,&ioctl_cfg);
            printf("The time now is %d\n",*(rtc_para.time));
            PrivTaskDelay(5000);  
        }
    }
    
    PrivClose(rtc_fd);
    return;
}

PRIV_SHELL_CMD_FUNCTION(TestRTC, a rtc test sample, PRIV_SHELL_CMD_MAIN_ATTR);
