/*
 * Copyright (c) 2017 Simon Goldschmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt
 *
 */
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
* @file sys_arch.c
* @brief In order to adapt to XiUOS, some changes have been made to implement the LwIP interface.
* @version 1.0
* @author AIIT XUOS Lab
* @date 2021-05-29
*/
#include "debug.h"

#include <lwip/opt.h>
#include <lwip/arch.h>

#include "tcpip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sio.h"
#include "ethernetif.h"

#if !NO_SYS
#include "sys_arch.h"
#endif
#include <lwip/stats.h>
#include <lwip/debug.h>
#include <lwip/sys.h>
#include "lwip/dhcp.h"
#include <string.h>
#include <xs_ktask.h>
#include <xs_ktick.h>
#include <xs_assign.h>
#include <xs_sem.h>
#include <xs_mutex.h>
#include <xs_ktask.h>
#include <xs_msg.h>
#include <ethernetif.h>

int errno;


x_ticks_t lwip_sys_now;

struct sys_timeouts {
  struct sys_timeo *next;
};

struct timeoutlist
{
	struct sys_timeouts timeouts;
	int32 pid;
};

#define SYS_THREAD_MAX 4

static struct timeoutlist s_timeoutlist[SYS_THREAD_MAX];

static u16_t s_nextthread = 0;

u32_t
sys_jiffies(void)
{
  lwip_sys_now = CurrentTicksGain();
  return lwip_sys_now;
}

u32_t
sys_now(void)
{
  lwip_sys_now = CurrentTicksGain();
  return lwip_sys_now;
}

void
sys_init(void)
{
	int i;
	for(i = 0; i < SYS_THREAD_MAX; i++)
	{
		s_timeoutlist[i].pid = 0;
		s_timeoutlist[i].timeouts.next = NULL;
	}
	s_nextthread = 0;
}

struct sys_timeouts *sys_arch_timeouts(void)
{
	int i;
	int32 pid;
	struct timeoutlist *tl;
	pid = (int32)GetKTaskDescriptor()->id.id;
	for(i = 0; i < s_nextthread; i++)
	{
		tl = &(s_timeoutlist[i]);
		if(tl->pid == pid)
		{
			return &(tl->timeouts);
		}
	}
	return NULL;
}

sys_prot_t sys_arch_protect(void)
{
	return CriticalAreaLock();
}

void sys_arch_unprotect(sys_prot_t pval)
{
	CriticalAreaUnLock(pval);
}

#if !NO_SYS

err_t
sys_sem_new(sys_sem_t *sem, u8_t count)
{
  *sem = KSemaphoreCreate((uint16)count);

#if SYS_STATS
	++lwip_stats.sys.sem.used;
 	if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used) {
		lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
	}
#endif /* SYS_STATS */

  if(*sem >= 0)
    return ERR_OK;
  else
  {
#if SYS_STATS
    ++lwip_stats.sys.sem.err;
#endif /* SYS_STATS */
    KPrintf("[sys_arch]:new sem fail!\n");
    return ERR_MEM;
  }
}

void
sys_sem_free(sys_sem_t *sem)
{
#if SYS_STATS
   --lwip_stats.sys.sem.used;
#endif /* SYS_STATS */
  KSemaphoreDelete(*sem);
  *sem = SYS_SEM_NULL;
}

int sys_sem_valid(sys_sem_t *sem)
{
  return (*sem >= SYS_SEM_NULL);
}

void
sys_sem_set_invalid(sys_sem_t *sem)
{
  *sem = SYS_SEM_NULL;
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  x_ticks_t start_tick = 0 ;
  int32 wait_time = 0;


  if(*sem == SYS_SEM_NULL)
    return SYS_ARCH_TIMEOUT;


  start_tick = CurrentTicksGain();

  if (0 == timeout)
    wait_time = WAITING_FOREVER;
  else
    wait_time = timeout;


  if(KSemaphoreObtain(*sem, wait_time) == EOK)
    return ((CurrentTicksGain()-start_tick)*MS_PER_SYSTICK_F407);
  else
    return SYS_ARCH_TIMEOUT;
}

void sys_sem_signal(sys_sem_t *sem)
{
  if(KSemaphoreAbandon( *sem ) != EOK)
    KPrintf("[sys_arch]:sem signal fail!\n");
}

err_t sys_mutex_new(sys_mutex_t *mutex)
{
  *mutex = KMutexCreate();
  if(*mutex >= 0)
    return ERR_OK;
  else
  {
    KPrintf("[sys_arch]:new mutex fail!\n");
    return ERR_MEM;
  }
}

void sys_mutex_free(sys_mutex_t *mutex)
{
  KMutexDelete(*mutex);
}

void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
  *mutex = SYS_MRTEX_NULL;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
  KMutexObtain(*mutex,
                 WAITING_FOREVER);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
  KMutexAbandon( *mutex );
}


sys_thread_t sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
  sys_thread_t handle = -1;
  handle =  KTaskCreate(name,
                         function,
                         arg,
                         (uint32)stacksize,
                         (uint8)prio);

  if (handle >= 0)
  {
    StartupKTask(handle);
    return handle;
  }
  return -ERROR;
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
  *mbox = KCreateMsgQueue(sizeof(void *), size);

#if SYS_STATS
      ++lwip_stats.sys.mbox.used;
      if (lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used) {
         lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
	  }
#endif /* SYS_STATS */
	if(*mbox < 0)
    return ERR_MEM;

  return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
  KDeleteMsgQueue(*mbox);
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
  if (*mbox < SYS_MBOX_NULL)
    return 0;
  else
    return 1;
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
  *mbox = SYS_MBOX_NULL;
}

void sys_mbox_post(sys_mbox_t *q, void *msg)
{
  while(KMsgQueueSendwait( *q, &msg, sizeof(void *), WAITING_FOREVER) != EOK);
}

err_t sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
  if(KMsgQueueSend(*q, &msg, sizeof(void *)) == EOK)
    return ERR_OK;
  else
    return ERR_MEM;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
  return sys_mbox_trypost(q, msg);
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)
{
  x_ticks_t start_tick = 0 ;
  int32 wait_time = 0;

  start_tick = CurrentTicksGain();

  if (0 == timeout)
    wait_time = WAITING_FOREVER;
  else
    wait_time = timeout;

  if(KMsgQueueRecv(*q, &(*msg), sizeof(void *), wait_time) == EOK)
    return ((CurrentTicksGain()-start_tick)*MS_PER_SYSTICK_F407);
  else{
    *msg = NULL;
    return SYS_ARCH_TIMEOUT;
  }
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
  if(KMsgQueueRecv(*q, &(*msg), sizeof(void *), 0) == EOK)
    return ERR_OK;
  else
    return SYS_MBOX_EMPTY;
}

#if LWIP_NETCONN_SEM_PER_THREAD
#error LWIP_NETCONN_SEM_PER_THREAD==1 not supported
#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#endif /* !NO_SYS */

/* Variables Initialization */
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;
uint8_t IP_ADDRESS[4];
uint8_t NETMASK_ADDRESS[4];
uint8_t GATEWAY_ADDRESS[4];

void TcpIpInit(void)
{
  tcpip_init(NULL, NULL);

  /* IP addresses initialization */
  /* USER CODE BEGIN 0 */
#if LWIP_DHCP
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
#else
  #ifdef SET_AS_SERVER
  IP4_ADDR(&ipaddr,IP_ADDR0_SERVER,IP_ADDR1_SERVER,IP_ADDR2_SERVER,IP_ADDR3_SERVER);
  #else
  IP4_ADDR(&ipaddr,IP_ADDR0_ClIENT,IP_ADDR1_ClIENT,IP_ADDR2_ClIENT,IP_ADDR3_ClIENT);
  #endif

  IP4_ADDR(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP4_ADDR(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* USE_DHCP */
  /* USER CODE END 0 */
  /* Initilialize the LwIP stack without RTOS */
  /* add the network interface (IPv4/IPv6) without RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    KPrintf("TcpIpInit : netif_set_up\n");
    netif_set_up(&gnetif);
  }
  else
  {
    /* When the netif link is down this function must be called */
    KPrintf("TcpIpInit : netif_set_down\n");
    netif_set_down(&gnetif);
  }

#if LWIP_DHCP
  int err;
  /*  Creates a new DHCP client for this interface on the first call.
  Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
  the predefined regular intervals after starting the client.
  You can peek in the netif->dhcp struct for the actual DHCP status.*/


  err = dhcp_start(&gnetif);
  if(err == ERR_OK)
    KPrintf("lwip dhcp init success...\n\n");
  else
    KPrintf("lwip dhcp init fail...\n\n");
  while(ip_addr_cmp(&(gnetif.ip_addr),&ipaddr))
  {
    vTaskDelay(1);
  }
#endif
  KPrintf("\n\nIP:%d.%d.%d.%d\n\n",  \
        ((gnetif.ip_addr.addr)&0x000000ff),       \
        (((gnetif.ip_addr.addr)&0x0000ff00)>>8),  \
        (((gnetif.ip_addr.addr)&0x00ff0000)>>16), \
        ((gnetif.ip_addr.addr)&0xff000000)>>24);
}

