/****************************************************************************
 * libs/libc-musl/math/lib_log2l.c
 *
 * This file is a part of NuttX:
 *
 *   Copyright (C) 2012 Gregory Nutt. All rights reserved.
 *   Ported by: Darcy Gong
 *
 * It derives from the Rhombus OS math library by Nick Johnson which has
 * a compatibile, MIT-style license:
 *
 * Copyright (C) 2009, 2010 Nick Johnson <nickbjohnson4224 at gmail.com>
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
long double log2l(long double x)
{
	return log2(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
/* Coefficients for ln(1+x) = x - x**2/2 + x**3 P(x)/Q(x)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 6.2e-22
 */
static const long double P[] = {
 4.9962495940332550844739E-1L,
 1.0767376367209449010438E1L,
 7.7671073698359539859595E1L,
 2.5620629828144409632571E2L,
 4.2401812743503691187826E2L,
 3.4258224542413922935104E2L,
 1.0747524399916215149070E2L,
};
static const long double Q[] = {
/* 1.0000000000000000000000E0,*/
 2.3479774160285863271658E1L,
 1.9444210022760132894510E2L,
 7.7952888181207260646090E2L,
 1.6911722418503949084863E3L,
 2.0307734695595183428202E3L,
 1.2695660352705325274404E3L,
 3.2242573199748645407652E2L,
};

/* Coefficients for log(x) = z + z^3 P(z^2)/Q(z^2),
 * where z = 2(x-1)/(x+1)
 * 1/sqrt(2) <= x < sqrt(2)
 * Theoretical peak relative error = 6.16e-22
 */
static const long double R[4] = {
 1.9757429581415468984296E-3L,
-7.1990767473014147232598E-1L,
 1.0777257190312272158094E1L,
-3.5717684488096787370998E1L,
};
static const long double S[4] = {
/* 1.00000000000000000000E0L,*/
-2.6201045551331104417768E1L,
 1.9361891836232102174846E2L,
-4.2861221385716144629696E2L,
};
/* log2(e) - 1 */
#define LOG2EA 4.4269504088896340735992e-1L

#define SQRTH 0.70710678118654752440L

long double log2l(long double x)
{
	long double y, z;
	int e;

	if (isnan(x))
		return x;
	if (x == INFINITY)
		return x;
	if (x <= 0.0) {
		if (x == 0.0)
			return -1/(x*x); /* -inf with divbyzero */
		return 0/0.0f; /* nan with invalid */
	}

	/* separate mantissa from exponent */
	/* Note, frexp is used so that denormal numbers
	 * will be handled properly.
	 */
	x = frexpl(x, &e);

	/* logarithm using log(x) = z + z**3 P(z)/Q(z),
	 * where z = 2(x-1)/x+1)
	 */
	if (e > 2 || e < -2) {
		if (x < SQRTH) {  /* 2(2x-1)/(2x+1) */
			e -= 1;
			z = x - 0.5;
			y = 0.5 * z + 0.5;
		} else {  /*  2 (x-1)/(x+1)   */
			z = x - 0.5;
			z -= 0.5;
			y = 0.5 * x + 0.5;
		}
		x = z / y;
		z = x*x;
		y = x * (z * __polevll(z, R, 3) / __p1evll(z, S, 3));
		goto done;
	}

	/* logarithm using log(1+x) = x - .5x**2 + x**3 P(x)/Q(x) */
	if (x < SQRTH) {
		e -= 1;
		x = 2.0*x - 1.0;
	} else {
		x = x - 1.0;
	}
	z = x*x;
	y = x * (z * __polevll(x, P, 6) / __p1evll(x, Q, 7));
	y = y - 0.5*z;

done:
	/* Multiply log of fraction by log2(e)
	 * and base 2 exponent by 1
	 *
	 * ***CAUTION***
	 *
	 * This sequence of operations is critical and it may
	 * be horribly defeated by some compiler optimizers.
	 */
	z = y * LOG2EA;
	z += x * LOG2EA;
	z += y;
	z += x;
	z += e;
	return z;
}
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384
// TODO: broken implementation to make things compile
long double log2l(long double x)
{
	return log2(x);
}
#endif


#endif
