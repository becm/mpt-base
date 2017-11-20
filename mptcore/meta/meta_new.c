/*!
 * default metatype
 */

#include <errno.h>
#include <string.h>

#include <sys/uio.h>

#include "array.h"
#include "convert.h"

#include "meta.h"

/*!
 * \ingroup mptMeta
 * \brief new metatype
 * 
 * Create matching text metatype for value.
 * Base implementation supports text content only
 * 
 * \param post  additional size for data
 * 
 * \return new metatype instance
 */
extern MPT_INTERFACE(metatype) *mpt_meta_new(MPT_STRUCT(value) val)
{
	MPT_INTERFACE(metatype) *mt;
	const char *src;
	size_t len;
	
	src = 0;
	len = 0;
	src = val.ptr;
	if (!val.fmt) {
		if (src) {
			len = strlen(src);
		}
	}
	else if (!val.fmt[0] || val.fmt[1]) {
		errno = EINVAL;
		return 0;
	}
	else if (!(src = mpt_data_tostring((const void **) &src, *val.fmt, &len))) {
		errno = EINVAL;
		return 0;
	}
	else if (val.fmt[1]) {
		errno = EINVAL;
		return 0;
	}
	/* data too big for basic type */
	if (len >= UINT8_MAX) {
		MPT_STRUCT(array) a = MPT_ARRAY_INIT;
		char *dest;
		
		/* include termination */
		if (src && !val.fmt) {
			++len;
		}
		if (!(dest = mpt_array_slice(&a, 0, len))) {
			return 0;
		}
		if (src) {
			memcpy(dest, src, len);
		} else {
			memset(dest, 0, len);
		}
		/* metatype with buffer text store */
		mt = (void *) mpt_meta_buffer(&a);
		mpt_array_clone(&a, 0);
		return mt;
	}
	if (!(mt = mpt_meta_geninfo(len))
	    || !src
	    || _mpt_geninfo_set(mt + 1, src, len) >= 0) {
		return mt;
	}
	mt->_vptr->ref.unref((void *) mt);
	errno = EINVAL;
	return 0;
}
