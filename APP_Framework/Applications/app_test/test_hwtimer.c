#include <stdio.h>
#include <string.h>
#include <transform.h>

#define BSP_LED_PIN 29
#define NULL_PARAMETER 0

static uint16_t pinval=0;
static uint16_t pin_fd=0;

void ledflip(void *parameter)
{
    struct PinStat pin_led;
    pin_led.pin = BSP_LED_PIN;
    pin_led.val = !pinval;
    pinval = !pinval;
    PrivWrite(pin_fd,&pin_led,NULL_PARAMETER);
    // printf("Timer has callback once:%d\n",pinval);
}

void TestHwTimer(void)
{
    x_ticks_t period = 100;//uint:10ms
    
    pin_fd = PrivOpen(HWTIMER_PIN_DEV_DRIVER, O_RDWR);
    if(pin_fd<0){
        printf("open pin fd error:%d\n",pin_fd);
        return;
    }

    //config led pin in board
     struct PinParam parameter;
     parameter.cmd = GPIO_CONFIG_MODE;
     parameter.pin = BSP_LED_PIN;
     parameter.mode = GPIO_CFG_OUTPUT;

    struct PrivIoctlCfg ioctl_cfg;
    ioctl_cfg.ioctl_driver_type = PIN_TYPE;
    ioctl_cfg.args =  (void *)&parameter;

    if (0 != PrivIoctl(pin_fd, OPE_CFG, &ioctl_cfg)) {
        printf("ioctl pin fd error %d\n", pin_fd);
        PrivClose(pin_fd);
        return;
    }

    int32 timer_handle = KCreateTimer("LED on and off by 1s",&ledflip,&pin_fd,period,TIMER_TRIGGER_PERIODIC);
    
    KTimerStartRun(timer_handle);
    
}

PRIV_SHELL_CMD_FUNCTION(TestHwTimer, a timer test sample, PRIV_SHELL_CMD_MAIN_ATTR);