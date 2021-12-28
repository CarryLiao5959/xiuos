/****************************************************************************
 * apps/nshlib/nsh_Applicationscmd.c
 *
 *   Copyright (C) 2016 Alan Carvalho de Assis. All rights reserved.
 *   Author: Alan Carvalho de Assis <acassis@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <errno.h>

#include "nsh.h"
#include "nsh_console.h"

extern int FrameworkInit(void);

/****************************************************************************
 * Name: cmd_Pm10Ps5308
 ****************************************************************************/

#if defined(CONFIG_APPLICATION_SENSOR_PM1_0_PS5308) && !defined(CONFIG_NSH_DISABLE_PM1_0PS5308)
extern void Pm10Ps5308(void);
int cmd_Pm10Ps5308(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    printf("Hello, world!\n");
    FrameworkInit();
    Pm10Ps5308();
    return 0;
}
#endif

/****************************************************************************
 * Name: cmd_Pm25Ps5308
 ****************************************************************************/

#if defined(CONFIG_APPLICATION_SENSOR_PM2_5_PS5308) && !defined(CONFIG_NSH_DISABLE_PM2_5PS5308)
extern void Pm25Ps5308(void);
int cmd_Pm25Ps5308(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    printf("Hello, world!\n");
    FrameworkInit();
    Pm25Ps5308();
    return 0;
}
#endif

/****************************************************************************
 * Name: cmd_Pm10Ps5308
 ****************************************************************************/

#if defined(CONFIG_APPLICATION_SENSOR_PM10_PS5308) && !defined(CONFIG_NSH_DISABLE_PM10PS5308)
extern void Pm100Ps5308(void);
int cmd_Pm100Ps5308(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    printf("Hello, world!\n");
    FrameworkInit();
    Pm100Ps5308();
    return 0;
}
#endif

/****************************************************************************
 * Name: cmd_TempHs300x
 ****************************************************************************/

#if defined(CONFIG_APPLICATION_SENSOR_TEMPERATURE_HS300X) && !defined(CONFIG_NSH_DISABLE_TEMPHS300X)
extern void TempHs300x(void);
int cmd_TempHs300x(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    printf("Hello, world!\n");
    FrameworkInit();
    TempHs300x();
    return 0;
}
#endif

/****************************************************************************
 * Name: cmd_HumiHs300x
 ****************************************************************************/

#if defined(CONFIG_APPLICATION_SENSOR_HUMIDITY_HS300X) && !defined(CONFIG_NSH_DISABLE_HUMIHS300X)
extern void HumiHs300x(void);
int cmd_HumiHs300x(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
    printf("Hello, world!\n");
    FrameworkInit();
    HumiHs300x();
    return 0;
}
#endif
