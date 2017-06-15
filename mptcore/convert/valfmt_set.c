/*!
 * set history state
 */

#include "meta.h"
#include "array.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief set value format
 * 
 * Set value formats from source.
 * 
 * \param arr  value format array
 * \param src  metatype descriptor
 * 
 * \return consumed length
 */
extern int mpt_valfmt_set(MPT_STRUCT(array) *arr, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it;
	MPT_STRUCT(array) tmp = MPT_ARRAY_INIT;
	MPT_STRUCT(valfmt) fmt;
	char *from;
	int ret;
	
	if (!src) {
		MPT_STRUCT(buffer) *buf;
		if (!(buf = arr->_buf)) {
			return 0;
		}
		if (buf->_vptr->content(buf) != MPT_ENUM(TypeValFmt)) {
			buf->_vptr->ref.unref((void *) buf);
			arr->_buf = 0;
			return 0;
		}
		return buf->_used / sizeof(MPT_STRUCT(valfmt));
	}
	if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeValFmt), &fmt)) >= 0
	    && (ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) >= 0
	    && it) {
		int curr;
		if ((curr = mpt_valfmt_add(&tmp, fmt)) < 0) {
			return MPT_ERROR(BadOperation);
		}
		ret = 0;
		src = (void *) it;
		while ((curr = it->_vptr->advance(it)) >= 0
		       && (curr = src->_vptr->conv(src, MPT_ENUM(TypeValFmt), &fmt)) >= 0) {
			if (mpt_valfmt_add(&tmp, fmt) < 0) {
				mpt_array_clone(&tmp, 0);
				return MPT_ERROR(BadOperation);
			}
			++ret;
		}
		mpt_array_clone(arr, &tmp);
		mpt_array_clone(&tmp, 0);
		return ret;
	}
	if ((ret = src->_vptr->conv(src, 's', &from)) < 0) {
		return ret;
	}
	else if (!from) {
		mpt_array_clone(arr, 0);
	}
	else {
		int curr;
		if ((curr = mpt_valfmt_parse(&tmp, from)) >= 0) {
			mpt_array_clone(arr, &tmp);
		}
		mpt_array_clone(&tmp, 0);
	}
	return 0;
}
