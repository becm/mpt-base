/*!
 * set point data from metatype.
 */

#include <sys/uio.h>

#include "meta.h"
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
	if (val->type == 's') {
		
	}
	if (val->type == MPT_ENUM(TypeIteratorPtr)) {
		MPT_INTERFACE(iterator) *it = *((void * const *) val->ptr);
		if (it) {
			double min = 0.0, max = 1.0;
			int len;
			if ((len = it->_vptr->get(it, 'd', &min)) < 0) {
				return len;
			}
			if (it->_vptr->advance(it) <= 0) {
				return MPT_ERROR(MissingData);
			}
			if (len > 0 && (len = it->_vptr->get(it, 'd', &max)) > 0) {
				if (it->_vptr->advance(it) < 0) {
					return MPT_ERROR(MissingData);
				}
				r->min = min;
				r->max = max;
				return 2;
			}
			return MPT_ERROR(MissingData);
		}
		r->min = 0.0;
		r->max = 1.0;
		return 0;
	}
	if (val->type == MPT_type_toVector('d')) {
		const struct iovec *vec = val->ptr;
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
