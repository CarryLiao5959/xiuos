/****************************************************************************
 * libs/libc-musl/math/lib_sqrtf.c
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
* @file lib_sqrtf.c
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

#include "libm.h"
#include <errno.h>

#include "sqrt_data.h"
#include "libc-musl.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifndef CONFIG_LIBM_ARCH_SQRTF

#define FENV_SUPPORT 1

static inline uint32_t mul32(uint32_t a, uint32_t b)
{
	return (uint64_t)a*b >> 32;
}

/* see sqrt.c for more detailed comments.  */

float sqrtf(float x)
{
	uint32_t ix, m, m1, m0, even, ey;

	ix = asuint(x);
	if (predict_false(ix - 0x00800000 >= 0x7f800000 - 0x00800000)) {
		/* x < 0x1p-126 or inf or nan.  */
		if (ix * 2 == 0)
			return x;
		if (ix == 0x7f800000)
			return x;
		if (ix > 0x7f800000)
			return __math_invalidf(x);
		/* x is subnormal, normalize it.  */
		ix = asuint(x * 0x1p23f);
		ix -= 23 << 23;
	}

	/* x = 4^e m; with int e and m in [1, 4).  */
	even = ix & 0x00800000;
	m1 = (ix << 8) | 0x80000000;
	m0 = (ix << 7) & 0x7fffffff;
	m = even ? m0 : m1;

	/* 2^e is the exponent part of the return value.  */
	ey = ix >> 1;
	ey += 0x3f800000 >> 1;
	ey &= 0x7f800000;

	/* compute r ~ 1/sqrt(m), s ~ sqrt(m) with 2 goldschmidt iterations.  */
	static const uint32_t three = 0xc0000000;
	uint32_t r, s, d, u, i;
	i = (ix >> 17) % 128;
	r = (uint32_t)__rsqrt_tab[i] << 16;
	/* |r*sqrt(m) - 1| < 0x1p-8 */
	s = mul32(m, r);
	/* |s/sqrt(m) - 1| < 0x1p-8 */
	d = mul32(s, r);
	u = three - d;
	r = mul32(r, u) << 1;
	/* |r*sqrt(m) - 1| < 0x1.7bp-16 */
	s = mul32(s, u) << 1;
	/* |s/sqrt(m) - 1| < 0x1.7bp-16 */
	d = mul32(s, r);
	u = three - d;
	s = mul32(s, u);
	/* -0x1.03p-28 < s/sqrt(m) - 1 < 0x1.fp-31 */
	s = (s - 1)>>6;
	/* s < sqrt(m) < s + 0x1.08p-23 */

	/* compute nearest rounded result.  */
	uint32_t d0, d1, d2;
	float y, t;
	d0 = (m << 16) - s*s;
	d1 = s - d0;
	d2 = d1 + s + 1;
	s += d1 >> 31;
	s &= 0x007fffff;
	s |= ey;
	y = asfloat(s);
	if (FENV_SUPPORT) {
		/* handle rounding and inexact exception. */
		uint32_t tiny = predict_false(d2==0) ? 0 : 0x01000000;
		tiny |= (d1^d2) & 0x80000000;
		t = asfloat(tiny);
		y = eval_as_float(y + t);
	}
	return y;
}


#endif