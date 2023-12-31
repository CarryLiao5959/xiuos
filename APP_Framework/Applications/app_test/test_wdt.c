#include <stdio.h>
#include <string.h>
#include <transform.h>

void TestWDT(int argc, char *agrv[])
{
    int wdt_fd = PrivOpen(WDT0_DEV_DRIVER, O_RDWR);
    if (wdt_fd < 0)
    {
        printf("open wdt_fd fd error:%d\n", wdt_fd);
        return;
    }
    printf("hw watchdog open!\n");

    // init watchdog
    int wdt_time = 1000;

    struct PrivIoctlCfg ioctl_cfg;
    ioctl_cfg.ioctl_driver_type = WDT_TYPE;
    ioctl_cfg.args = (void *)&wdt_time;

    if (0 != PrivIoctl(wdt_fd, OPER_WDT_SET_TIMEOUT, &ioctl_cfg))
    {
        printf("ioctl wdt fd error %d\n", wdt_fd);
        PrivClose(wdt_fd);
        return;
    }

    int test_counter = 100;

    // wdt feed or not according to argc,if argc!=1 then dog won't be feed
    while (test_counter--)
    {
        if (1 == argc)
        {
            printf("dog is feed\n");
            PrivIoctl(wdt_fd, OPER_WDT_KEEPALIVE, &ioctl_cfg); // feed dog
        }
        PrivTaskDelay(100);
    }
    PrivClose(wdt_fd);
    return;
}

PRIV_SHELL_CMD_FUNCTION(TestWDT, a wdt test sample, PRIV_SHELL_CMD_MAIN_ATTR);