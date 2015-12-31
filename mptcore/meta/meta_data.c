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
extern const char *mpt_meta_data(MPT_INTERFACE(metatype) *meta, size_t *len)
{
	struct iovec vec;
	const char *base;
	
	if (len && meta->_vptr->conv(meta, 'c' - 0x40, &vec) >= 0) {
		*len = vec.iov_len;
		return vec.iov_base;
	}
	if (meta->_vptr->conv(meta, 's', &base) >= 0) {
		if (len) *len = base ? strlen(base) : 0;
		return base;
	}
	return 0;
}
