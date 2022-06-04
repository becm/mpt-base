/*!
 * default metatype
 */

#include <errno.h>
#include <string.h>

#include <sys/uio.h>

#include "array.h"
#include "convert.h"
#include "types.h"

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
extern MPT_INTERFACE(metatype) *mpt_meta_new(const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(metatype) *mt;
	const void *src;
	const char *text;
	size_t len;
	
	if (!val->type) {
		errno = EINVAL;
		return 0;
	}
	src = val->ptr;
	if (!(text = mpt_data_tostring(&src, val->type, &len))) {
		/* TODO generic type representation */
		errno = EINVAL;
		return 0;
	}
	/* data too big for basic type */
	if (len >= UINT8_MAX) {
		MPT_STRUCT(array) a = MPT_ARRAY_INIT;
		MPT_STRUCT(buffer) *buf;
		const MPT_STRUCT(type_traits) *traits;
		size_t reserve;
		
		if (!(traits = mpt_type_traits('c'))) {
			errno = EINVAL;
			return 0;
		}
		/* create new text data array */
		reserve = (len && text[len - 1]) ? len + 1 : len;
		if (!(buf = mpt_array_reserve(&a, reserve, traits))) {
			return 0;
		}
		if (!mpt_buffer_set(buf, traits, len, text, 0)
		 || ((reserve > len) && !mpt_buffer_set(buf, traits, 1, "", len))) {
			mpt_array_clone(&a, 0);
			return 0;
		}
		/* metatype with buffer text store */
		mt = mpt_meta_buffer(&a);
		mpt_array_clone(&a, 0);
		return mt;
	}
	if (!(mt = mpt_meta_geninfo(len))
	    || !src
	    || _mpt_geninfo_set(mt + 1, text, len) >= 0) {
		return mt;
	}
	mt->_vptr->unref(mt);
	errno = EINVAL;
	return 0;
}
