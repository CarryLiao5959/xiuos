/****************************************************************************
 * libs/libc-musl/math/lib_acoshl.c
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

#ifdef CONFIG_HAVE_LONG_DOUBLE

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double acoshl(long double x)
{
	return acosh(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/* acosh(x) = log(x + sqrt(x*x-1)) */
long double acoshl(long double x)
{
	union ldshape u = {x};
	int e = u.i.se & 0x7fff;

	if (e < 0x3fff + 1)
		/* |x| < 2, invalid if x < 1 or nan */
		return log1pl(x-1 + sqrtl((x-1)*(x-1)+2*(x-1)));
	if (e < 0x3fff + 32)
		/* |x| < 0x1p32 */
		return logl(2*x - 1/(x+sqrtl(x*x-1)));
	return logl(x) + 0.693147180559945309417232121458176568L;
}
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384
// TODO: broken implementation to make things compile
long double acoshl(long double x)
{
	return acosh(x);
}
#endif

#endif
