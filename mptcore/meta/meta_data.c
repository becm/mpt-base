/*!
 * get data from metatype.
 */

#include <string.h>
#include <sys/uio.h>

#include "convert.h"

/*!
 * \ingroup mptMeta
 * \brief get metatype data
 * 
 * Try to get data from metatype via
 *  1) cast to string (or vector)
 *  2) generic property
 * 
 * \param      meta data source
 * \param[out] len  length of raw data
 * 
 * \return start of string/raw data
 */
extern const void *mpt_meta_data(MPT_INTERFACE(metatype) *meta, size_t *len)
{
	MPT_STRUCT(property) pr;
	const struct iovec *vec;
	const char *base;
	int pos;
	
	if (len && (vec = meta->_vptr->typecast(meta, MPT_ENUM(TypeVector)))) {
		*len = vec->iov_len;
		return vec->iov_base;
	}
	if ((base = meta->_vptr->typecast(meta, 's'))) {
		if (len) {
			*len = base ? strlen(base) : 0;
		}
		return base;
	}
	pr.name = "\0";
	
	if (meta->_vptr->property(meta, &pr, 0) < 0) {
		return 0;
	}
	base = pr.val.ptr;
	if (!pr.val.fmt) {
		if (len) *len = 0;
		return base;
	}
	if (len
	    && (pos = mpt_position(pr.val.fmt, MPT_ENUM(TypeVector))) >= 0
	    && (pos = mpt_offset(pr.val.fmt, pos)) >= 0) {
		vec = (void *) (base + pos);
		*len = vec->iov_len;
		return vec->iov_base;
	}
	if ((pos = mpt_position(pr.val.fmt, 's')) >= 0
	    && (pos = mpt_offset(pr.val.fmt, pos)) >= 0) {
		base = *((void **) (base + pos));
		if (len) {
			static const char def[] = "\0";
			if (base) {
				*len = strlen(base);
			} else {
				base = def;
				*len = 0;
			}
		}
		return base;
	}
	return 0;
}
