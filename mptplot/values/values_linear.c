/*!
 * set values according to linear type parameters
 */

#include <errno.h>

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief create linear profile
 * 
 * Set data to linear spaced values between \min and \max
 * 
 * \param points number of elements to set
 * \param target address of elements to set
 * \param ld     element advance
 * \param min    value at profile start
 * \param max    value at profile end
 * 
 * \return zero on success
 */
extern int mpt_values_linear(int points, double *target, int ld, double min, double max)
{
	int	i, len;
	double	dv;
	
	if (!target) {
		errno = EFAULT; return -1;
	}
	if (points < 1) {
		errno = ERANGE; return -1;
	}
	
	dv = (max - min)/(len = points-1);
	
	target[0] = min;
	
	for (i = 1; i < len; i++) {
		target[i*ld] = min + i * dv;
	}
	target[len*ld] = max;
	
	return 0;
}

