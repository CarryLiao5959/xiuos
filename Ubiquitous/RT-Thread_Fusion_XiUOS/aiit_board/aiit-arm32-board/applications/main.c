/*
 * @Author: chunyexixiaoyu
 * @Date: 2021-09-24 16:33:15
 * @LastEditTime: 2021-09-24 15:48:30
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \xiuos\Ubiquitous\RT_Thread\bsp\stm32f407-atk-coreboard\applications\main.c
 */

/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#include <board.h>
#include <stdio.h>
#include <string.h>
#ifdef RT_USING_POSIX
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <dfs_poll.h>
#include <dfs_posix.h>
#include <dfs.h>
#ifdef RT_USING_POSIX_TERMIOS
#include <posix_termios.h>
#endif
#endif

#define LED0_PIN    GET_PIN(G, 15)
extern int FrameworkInit(void);
int main(void)
{
     int count = 1;
     rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
     rt_thread_mdelay(100);
     FrameworkInit();
     printf("XIUOS stm32f4 build %s %s\n",__DATE__,__TIME__);        

    #ifdef APPLICATION_SENSOR_ALTITUDE_BMP180   
    extern int AltitudeBmp180();
    AltitudeBmp180();
    #endif

    #ifdef APPLICATION_SENSOR_CH4_AS830
    extern void Ch4As830();
    Ch4As830();
    #endif

    #ifdef APPLICATION_SENSOR_TVOC_TB600B_TVOC10
    extern void TvocTb600bTvoc10();
    TvocTb600bTvoc10();
    #endif

    #ifdef APPLICATION_SENSOR_CO2_G8S
    extern void Co2G8s();
    printf("Waiting for Co2G8s to start...\n");
    rt_thread_mdelay(9000);
    Co2G8s();
    #endif

    #ifdef APPLICATION_SENSOR_TEMPERATURE_HS300X
    extern void TempHs300x();
    TempHs300x();
    #endif

    #ifdef APPLICATION_SENSOR_HUMIDITY_HS300X
    extern void HumiHs300x();
    HumiHs300x();
    #endif

    #ifdef APPLICATION_SENSOR_HCHO_TB600B_WQ_HCHO1OS
    extern void HchoTb600bHcho1os();
    printf("Waiting for HchoTb600bHcho1os to start...\n");
    rt_thread_mdelay(10000);
    HchoTb600bHcho1os();
    #endif

    #ifdef APPLICATION_SENSOR_IAQ_TB600B_IAQ10
    extern void IaqTb600bIaq10();
    printf("Waiting for IaqTb600bIaq10 to start...\n");
    rt_thread_mdelay(9000);
    IaqTb600bIaq10();
    #endif

     while (count++)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
