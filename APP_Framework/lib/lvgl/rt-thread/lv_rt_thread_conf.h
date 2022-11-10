/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: MIT
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-15     Meco Man     The first version
 */

#ifndef LV_RT_THREAD_CONF_H
#define LV_RT_THREAD_CONF_H

#ifdef __RTTHREAD__

#define LV_RTTHREAD_INCLUDE <rtthread.h>
#include LV_RTTHREAD_INCLUDE

#ifdef __ARMCC_VERSION
#define lv_snprintf  rt_snprintf
#define lv_vsnprintf rt_vsnprintf
#endif



/*--END OF LV_RT_THREAD_CONF_H--*/

#endif /*__RTTHREAD__*/

#endif /*LV_CONF_H*/
