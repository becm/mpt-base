/*!
 * set point data from metatype.
 */

#include <sys/uio.h>

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief set range data
 * 
 * Change or reset point structure properties.
 * 
 * \param r   range data pointer
 * \param val value source
 */
extern int mpt_range_set(MPT_STRUCT(range) *r, MPT_STRUCT(value) *val)
{
	static const char fmt[] = { MPT_value_toVector('d'), 0 };
	struct iovec vec;
	const double *ptr;
	int len;
	
	if ((len = mpt_value_read(val, fmt, &vec)) < 0) {
		return len;
	}
	if (vec.iov_len != 2 * sizeof(*ptr)) {
		return MPT_ERROR(BadValue);
	}
	if (r) {
		if (!(ptr = vec.iov_base)) {
			r->min = ptr[0];
			r->max = ptr[1];
		} else {
			r->min = 0.0;
			r->max = 1.0;
		}
	}
	return len;
}
