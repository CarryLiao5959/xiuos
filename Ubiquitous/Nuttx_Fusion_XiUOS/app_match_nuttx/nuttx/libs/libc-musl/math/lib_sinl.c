/****************************************************************************
 * libs/libc-musl/math/lib_sinl.c
 *
 * This file is a part of NuttX:
 *
 *   Copyright (C) 2012 Gregory Nutt. All rights reserved.
 *   Ported by: Darcy Gong
 *
 * It derives from the Rhombus OS math library by Nick Johnson which has
 * a compatibile, MIT-style license:
 *
 * Copyright (C) 2009-2011 Nick Johnson <nickbjohnson4224 at gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 ****************************************************************************/

/**
* @file lib_sinl.c
* @brief nuttx source code
*        https://github.com/apache/incubator-nuttx.git
* @version 10.3.0 
* @author AIIT XUOS Lab
* @date 2022-08-04
*/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <sys/types.h>
#include "libm.h"

#ifdef CONFIG_HAVE_LONG_DOUBLE

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double sinl(long double x)
{
	return sin(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double sinl(long double x)
{
	union ldshape u = {x};
	unsigned n;
	long double y[2], hi, lo;

	u.i.se &= 0x7fff;
	if (u.i.se == 0x7fff)
		return x - x;
	if (u.f < M_PI_4) {
		if (u.i.se < 0x3fff - LDBL_MANT_DIG/2) {
			/* raise inexact if x!=0 and underflow if subnormal */
			FORCE_EVAL(u.i.se == 0 ? x*0x1p-120f : x+0x1p120f);
			return x;
		}
		return __sinl(x, 0.0, 0);
	}
	n = __rem_pio2l(x, y);
	hi = y[0];
	lo = y[1];
	switch (n & 3) {
	case 0:
		return __sinl(hi, lo, 1);
	case 1:
		return __cosl(hi, lo);
	case 2:
		return -__sinl(hi, lo, 1);
	case 3:
	default:
		return -__cosl(hi, lo);
	}
}
#endif
#endif
