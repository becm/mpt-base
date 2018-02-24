/*!
 * set values according to boundary type parameters
 */

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief create boundary profile
 * 
 * Set data to start,intermediate,end value.
 * 
 * \param points number of elements to set
 * \param target address of elements to set
 * \param ld     element advance
 * \param left   value at profile start
 * \param cont   values between first and last
 * \param right  value at profile end
 * 
 * \return zero on success
 */
extern void mpt_values_bound(long points, double *target, long ld, double left, double cont, double right)
{
	long i, end = points - 1;
	
	if (!target || points < 1) {
		return;
	}
	if (points < 2) {
		target[0] = (left + cont + right) / 3;
		return;
	}
	target[0] = left;
	
	for (i = 1; i < end; i++) {
		target[i * ld] = cont;
	}
	target[ld * end] = right;
}
