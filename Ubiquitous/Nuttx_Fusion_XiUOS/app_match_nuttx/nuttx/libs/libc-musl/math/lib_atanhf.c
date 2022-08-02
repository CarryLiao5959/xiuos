/****************************************************************************
 * libs/libc-musl/math/lib_atanhf.c
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
#include <nuttx/compiler.h>

#include "libm.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

float atanhf(float x)
{
	union {float f; uint32_t i;} u = {.f = x};
	unsigned s = u.i >> 31;
	float_t y;

	/* |x| */
	u.i &= 0x7fffffff;
	y = u.f;

	if (u.i < 0x3f800000 - (1<<23)) {
		if (u.i < 0x3f800000 - (32<<23)) {
			/* handle underflow */
			if (u.i < (1<<23))
				FORCE_EVAL((float)(y*y));
		} else {
			/* |x| < 0.5, up to 1.7ulp error */
			y = 0.5f*log1pf(2*y + 2*y*y/(1-y));
		}
	} else {
		/* avoid overflow */
		y = 0.5f*log1pf(2*(y/(1-y)));
	}
	return s ? -y : y;
}
