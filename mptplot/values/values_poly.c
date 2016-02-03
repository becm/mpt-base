/*!
 * create values from grid points and polynom parameter
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "convert.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief create polynom profile
 * 
 * Set data to result of polynom given according value from reference data.
 * 
 * \param points number of elements to set
 * \param target address of elements to set
 * \param ld     element advance
 * \param desc   polynam description
 * \param src    reference data
 * 
 * \return zero on success
 */
extern int mpt_values_poly(const char *descr, int points, double *target, int ld, const double *src)
{
	double coeff[128], shift[127];
	int i, nc = 0, ns = sizeof(coeff)/sizeof(*coeff);
	
	/* polynom coefficients */
	do {
		ssize_t len = mpt_cdouble(coeff+nc, descr, 0);
		
		if (len <= 0) {
			if (!nc) {
				return -2;
			}
			break;
		}
		descr += len;
	} while (++nc < ns);
	
	/* variable shift */
	if (!nc || !descr || !(descr = strchr(descr, ':'))) {
		ns = 0;
	}
	else if ((ns = mpt_values_string(descr+1, nc-1, shift, 1)) < 0) {
		return ns;
	}
	if (points && nc && (!target)) {
		return MPT_ERROR(BadArgument);
	}
	
	for (i = 0; i < points; i++) {
		double sum;
		int j;
		for (j = 0, sum = 0.0; j < nc; j++ ) {
			double prod, tmp = src? src[i] : i;
			int k;
			
			if (j < ns) tmp += shift[j];
			
			for (k = j + 1, prod = coeff[j]; k < nc; k++) {
				prod *= tmp;
			}
			sum += prod;
		}
		target[i*ld] = sum;
	}
	return 0;
}

