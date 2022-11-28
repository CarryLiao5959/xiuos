/****************************************************************************
 * apps/examples/fb/fb_main.c
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

#include <transform.h>

#ifdef ADD_NUTTX_FETURES

#ifdef CONFIG_BSP_USING_TOUCH
void TestTouch(void)
{
  int fd;
	TouchDataParam point = {0, 0, 0};
	fd = PrivOpen("/dev/touch_dev", O_RDWR);
	while(1)
	{
		PrivRead(fd,&point,1);
		printf("Now touch point:(%d,%d)\n",point.x,point.y);
	}
}
#endif

#endif
