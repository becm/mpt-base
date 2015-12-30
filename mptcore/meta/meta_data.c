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
	const struct iovec *vec;
	const char *base;
	
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
	if ((vec = meta->_vptr->typecast(meta, (char) ('c' | MPT_ENUM(TypeVector))))) {
		if (len) {
			*len = vec->iov_len;
		}
		return vec->iov_base;
	}
	return 0;
}
