/****************************************************************************
 * net/udp/udp_txdrain.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <sched.h>
#include <assert.h>
#include <errno.h>

#include <nuttx/semaphore.h>

#include "udp/udp.h"

#if defined(CONFIG_NET_UDP_WRITE_BUFFERS) && defined(CONFIG_NET_UDP_NOTIFIER)

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: txdrain_worker
 *
 * Description:
 *   Called with the write buffers have all been sent.
 *
 * Input Parameters:
 *   arg     - The notifier entry.
 *
 * Returned Value:
 *   None.
 *
 ****************************************************************************/

static void txdrain_worker(FAR void *arg)
{
  FAR sem_t *waitsem = (FAR sem_t *)arg;

  DEBUGASSERT(waitsem != NULL);

  /* Then just post the semaphore, waking up tcp_txdrain() */

  nxsem_post(waitsem);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: udp_txdrain
 *
 * Description:
 *   Wait for all write buffers to be sent (or for a timeout to occur).
 *
 * Input Parameters:
 *   psock   - An instance of the internal socket structure.
 *   timeout - The relative time when the timeout will occur
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned
 *   on any failure.
 *
 ****************************************************************************/

int udp_txdrain(FAR struct socket *psock, unsigned int timeout)
{
  FAR struct udp_conn_s *conn;
  sem_t waitsem;
  int ret;

  DEBUGASSERT(psock != NULL && psock->s_conn != NULL);
  DEBUGASSERT(psock->s_type == SOCK_DGRAM);

  conn = (FAR struct udp_conn_s *)psock->s_conn;

  /* Initialize the wait semaphore */

  nxsem_init(&waitsem, 0, 0);
  nxsem_set_protocol(&waitsem, SEM_PRIO_NONE);

  /* The following needs to be done with the network stable */

  net_lock();
  ret = udp_writebuffer_notifier_setup(txdrain_worker, conn, &waitsem);
  if (ret > 0)
    {
      int key = ret;

      /* There is pending write data.. wait for it to drain. */

      ret = net_timedwait_uninterruptible(&waitsem, timeout);

      /* Tear down the notifier (in case we timed out or were canceled) */

      if (ret < 0)
        {
          udp_notifier_teardown(key);
        }
    }

  net_unlock();
  nxsem_destroy(&waitsem);
  return ret;
}

#endif /* CONFIG_NET_UDP_WRITE_BUFFERS && CONFIG_NET_UDP_NOTIFIER */
