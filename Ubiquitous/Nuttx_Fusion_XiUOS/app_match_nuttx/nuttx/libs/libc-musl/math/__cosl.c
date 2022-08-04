/**
 * @file __cosl.c
 * @brief musl source code
 *        https://github.com/bminor/musl.git
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2022-08-04
 */

/* origin: FreeBSD /usr/src/lib/msun/ld80/k_cosl.c */
/* origin: FreeBSD /usr/src/lib/msun/ld128/k_cosl.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */


#include "libm.h"

#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
/*
 * ld80 version of __cos.c.  See __cos.c for most comments.
 */
/*
 * Domain [-0.7854, 0.7854], range ~[-2.43e-23, 2.425e-23]:
 * |cos(x) - c(x)| < 2**-75.1
 *
 * The coefficients of c(x) were generated by a pari-gp script using
 * a Remez algorithm that searches for the best higher coefficients
 * after rounding leading coefficients to a specified precision.
 *
 * Simpler methods like Chebyshev or basic Remez barely suffice for
 * cos() in 64-bit precision, because we want the coefficient of x^2
 * to be precisely -0.5 so that multiplying by it is exact, and plain
 * rounding of the coefficients of a good polynomial approximation only
 * gives this up to about 64-bit precision.  Plain rounding also gives
 * a mediocre approximation for the coefficient of x^4, but a rounding
 * error of 0.5 ulps for this coefficient would only contribute ~0.01
 * ulps to the final error, so this is unimportant.  Rounding errors in
 * higher coefficients are even less important.
 *
 * In fact, coefficients above the x^4 one only need to have 53-bit
 * precision, and this is more efficient.  We get this optimization
 * almost for free from the complications needed to search for the best
 * higher coefficients.
 */
static const long double
C1 =  0.0416666666666666666136L;        /*  0xaaaaaaaaaaaaaa9b.0p-68 */
static const double
C2 = -0.0013888888888888874,            /* -0x16c16c16c16c10.0p-62 */
C3 =  0.000024801587301571716,          /*  0x1a01a01a018e22.0p-68 */
C4 = -0.00000027557319215507120,        /* -0x127e4fb7602f22.0p-74 */
C5 =  0.0000000020876754400407278,      /*  0x11eed8caaeccf1.0p-81 */
C6 = -1.1470297442401303e-11,           /* -0x19393412bd1529.0p-89 */
C7 =  4.7383039476436467e-14;           /*  0x1aac9d9af5c43e.0p-97 */
#define POLY(z) (z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*(C6+z*C7)))))))
#elif LDBL_MANT_DIG == 113
/*
 * ld128 version of __cos.c.  See __cos.c for most comments.
 */
/*
 * Domain [-0.7854, 0.7854], range ~[-1.80e-37, 1.79e-37]:
 * |cos(x) - c(x))| < 2**-122.0
 *
 * 113-bit precision requires more care than 64-bit precision, since
 * simple methods give a minimax polynomial with coefficient for x^2
 * that is 1 ulp below 0.5, but we want it to be precisely 0.5.  See
 * above for more details.
 */
static const long double
C1 =  0.04166666666666666666666666666666658424671L,
C2 = -0.001388888888888888888888888888863490893732L,
C3 =  0.00002480158730158730158730158600795304914210L,
C4 = -0.2755731922398589065255474947078934284324e-6L,
C5 =  0.2087675698786809897659225313136400793948e-8L,
C6 = -0.1147074559772972315817149986812031204775e-10L,
C7 =  0.4779477332386808976875457937252120293400e-13L;
static const double
C8 = -0.1561920696721507929516718307820958119868e-15,
C9 =  0.4110317413744594971475941557607804508039e-18,
C10 = -0.8896592467191938803288521958313920156409e-21,
C11 =  0.1601061435794535138244346256065192782581e-23;
#define POLY(z) (z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*(C6+z*(C7+ \
	z*(C8+z*(C9+z*(C10+z*C11)))))))))))
#endif

long double __cosl(long double x, long double y)
{
	long double hz,z,r,w;

	z  = x*x;
	r  = POLY(z);
	hz = 0.5*z;
	w  = 1.0-hz;
	return w + (((1.0-w)-hz) + (z*r-x*y));
}
#endif
