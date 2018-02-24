/*!
 * set values according to linear type parameters
 */

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
extern void mpt_values_linear(long points, double *target, long ld, double min, double max)
{
	long i, len;
	double dv;
	
	if (!target || points < 1) {
		return;
	}
	dv = (max - min) / (len = points - 1);
	
	target[0] = min;
	
	for (i = 1; i < len; i++) {
		target[i * ld] = min + i * dv;
	}
	target[len * ld] = max;
}
