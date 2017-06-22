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
	MPT_STRUCT(value) val = MPT_VALUE_INIT;
	MPT_STRUCT(valfmt) fmt;
	int ret, curr;
	
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
	/* get elements from iterator */
	it = 0;
	if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) > 0) {
		if ((curr = mpt_valfmt_add(&tmp, fmt)) < 0) {
			return MPT_ERROR(BadOperation);
		}
		ret = 0;
		while ((curr = it->_vptr->get(it, MPT_ENUM(TypeValFmt), &fmt)) > 0) {
			if (it->_vptr->advance(it) < 0) {
				break;
			}
			if (mpt_valfmt_add(&tmp, fmt) < 0) {
				mpt_array_clone(&tmp, 0);
				return MPT_ERROR(BadOperation);
			}
			++ret;
		}
	}
	/* get elements from value */
	else if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeValue), &val)) > 0 && val.fmt) {
		static const char valfmt[] = { MPT_ENUM(TypeValFmt), 0 };
		if (!val.ptr) {
			return MPT_ERROR(BadValue);
		}
		ret = 0;
		while ((curr = mpt_value_read(&val, valfmt, &fmt))) {
			if (curr < 0) {
				mpt_array_clone(&tmp, 0);
				return MPT_ERROR(BadType);
			}
			if (mpt_valfmt_add(&tmp, fmt) < 0) {
				mpt_array_clone(&tmp, 0);
				return MPT_ERROR(BadOperation);
			}
			++ret;
		}
	}
	/* get elements from string */
	else {
		char *from;
		
		if ((ret = src->_vptr->conv(src, 's', &from)) < 0) {
			return ret;
		}
		else if (!from) {
			mpt_array_clone(arr, 0);
			return 0;
		}
		else if ((curr = mpt_valfmt_parse(&tmp, from)) < 0) {
			return curr;
		}
		ret = 0;
	}
	mpt_array_clone(arr, &tmp);
	mpt_array_clone(&tmp, 0);
	return ret;
}
