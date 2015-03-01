/*!
 * set values according to boundary type parameters
 */

#include <stdlib.h>
#include <errno.h>

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
extern int mpt_values_bound(int points, double *target, int ld, double left, double cont, double right)
{
	int i, end = points - 1;
	
	if (!target) {
		errno = EFAULT; return -1;
	}
	if (points < 1) {
		errno = ERANGE; return -1;
	}
	
	target[0] = left;
	
	for (i = 1; i < end; i++) {
		target[i*ld] = cont;
	}
	target[ld*end] = right;
	
	return 0;
}

