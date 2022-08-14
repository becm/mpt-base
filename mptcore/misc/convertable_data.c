/*!
 * get data from metatype.
 */

#include <string.h>
#include <sys/uio.h>

#include "types.h"

#include "core.h"

/*!
 * \ingroup mptCore
 * \brief get string from convertable data
 * 
 * Try to get text data from metatype via
 *  1) character vector (if len pointer supplied)
 *  2) generic string pointer
 * 
 * \param      val data source
 * \param[out] len length of raw data
 * 
 * \return start of string
 */
extern const char *mpt_convertable_data(MPT_INTERFACE(convertable) *val, size_t *len)
{
	struct iovec vec;
	const char *base;
	
	if (len && val->_vptr->convert(val, MPT_type_toVector('c'), &vec) >= 0) {
		*len = vec.iov_len;
		return vec.iov_base;
	}
	if (val->_vptr->convert(val, 's', &base) >= 0) {
		if (len) *len = base ? strlen(base) : 0;
		return base;
	}
	return 0;
}
