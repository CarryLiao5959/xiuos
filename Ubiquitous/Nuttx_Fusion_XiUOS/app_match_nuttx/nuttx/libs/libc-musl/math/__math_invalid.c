/**
 * @file __math_invalid.c
 * @brief musl source code
 *        https://github.com/bminor/musl.git
 * @version 1.0
 * @author AIIT XUOS Lab
 * @date 2022-08-04
 */

#include "libm.h"

double __math_invalid(double x)
{
	return (x - x) / (x - x);
}
