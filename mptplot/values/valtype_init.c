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
extern int mpt_valtype_init(int type, const char *descr, int len, double *data, int ld, const double *ref)
{
	double param[3] = { 0., 1., 0. };
	
	switch (type) {
		case 0: /* no real profile */
			return mpt_values_bound(len, data, ld, 0., 0., 0.);
		case MPT_ENUM(ValueFormatLinear):
			(void) mpt_values_string(descr, 2, param, 1);
			return mpt_values_linear(len, data, ld, param[0], param[1]);
		case MPT_ENUM(ValueFormatBoundaries):
			(void) mpt_values_string(descr, 3, param, 1);
			return mpt_values_bound(len, data, ld, param[0], param[1], param[2]);
		case MPT_ENUM(ValueFormatPolynom):
			return mpt_values_poly(descr, len, data, ld, ref);
		case MPT_ENUM(ValueFormatText):
			type = mpt_values_string(descr, len, data, ld);
			if (type >= 0 && type < len) {
				double def;
				int i;
				
				def = type ? data[type-1] : 0.;
				
				for (i = type; i < len; i++) {
					data[i] = def;
				}
			}
			return type;
		default:
			return MPT_ERROR(BadType);
	}
}

