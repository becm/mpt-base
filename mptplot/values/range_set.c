/*!
 * set point data from metatype.
 */

#include <sys/uio.h>

#include "types.h"

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
extern int mpt_range_set(MPT_STRUCT(range) *r, const MPT_STRUCT(value) *val)
{
	if (!MPT_value_isBaseType(val)) {
		return MPT_ERROR(BadArgument);
	}
	if (val->_type == 's') {
		
	}
	if (val->_type == MPT_ENUM(TypeIteratorPtr)) {
		MPT_INTERFACE(iterator) *it = *((void * const *) val->_addr);
		if (it) {
			double min = 0.0, max = 1.0;
			int len;
			if ((len = mpt_iterator_consume(it, 'd', &min)) < 0) {
				return len;
			}
			if (!len) {
				return MPT_ERROR(MissingData);
			}
			if ((len = mpt_iterator_consume(it, 'd', &max)) < 0) {
				return len;
			}
			r->min = min;
			r->max = max;
			return 2;
		}
		r->min = 0.0;
		r->max = 1.0;
		return 0;
	}
	if (val->_type == MPT_type_toVector('d')) {
		const struct iovec *vec = val->_addr;
		if (vec && (vec->iov_len / sizeof(double) == 2)) {
			const double *ptr;
			if ((ptr = vec->iov_base)) {
				r->min = ptr[0];
				r->max = ptr[1];
			} else {
				r->min = 0.0;
				r->max = 1.0;
			}
			return 0;
		}
		return MPT_ERROR(BadValue);
	}
	return MPT_ERROR(BadType);
}
