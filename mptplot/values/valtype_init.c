/*!
 * set vector data from description and reference points
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief initialize values
 * 
 * Set values according to description.
 * 
 * \param len   number of elements
 * \param data  data to set
 * \param ld    element distance
 * \param descr value description
 * \param type  type of description
 * \param ref   reference values (for polynom type)
 * 
 * \return result of data set
 */
extern int mpt_valtype_init(int len, double *data, int ld, const char *descr, int type, const double *ref)
{
	double param[3] = { 0., 1., 0. };
	
	switch (type) {
		case 0: /* no real profile */
			return mpt_values_bound(len, data, ld, 0., 0., 0.);
		case MPT_ENUM(ValueFormatLinear):
			(void) mpt_values_string(2, param, 1, descr);
			return mpt_values_linear(len, data, ld, param[0], param[1]);
		case MPT_ENUM(ValueFormatBoundaries):
			(void) mpt_values_string(3, param, 1, descr);
			return mpt_values_bound(len, data, ld, param[0], param[1], param[2]);
		case MPT_ENUM(ValueFormatPolynom):
			return mpt_values_poly(len, data, ld, descr, ref);
		case MPT_ENUM(ValueFormatText):
			type = mpt_values_string(len, data, ld, descr);
			if (type >= 0 && type < len) {
				int i;
				
				param[0] = type ? data[type-1] : 0.;
				
				for (i = type; i < len; i++) {
					data[i] = param[0];
				}
			}
			return type;
		default:
			errno = EINVAL; return -1;
	}
}

