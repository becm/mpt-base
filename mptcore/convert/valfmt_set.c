/*!
 * MPT core library
 *   create value format data
 */

#include "meta.h"
#include "array.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief set value format
 * 
 * Set value format elements from source.
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
	MPT_STRUCT(value_format) fmt;
	int ret, curr;
	
	if (!src) {
		const MPT_STRUCT(type_traits) *info;
		const MPT_STRUCT(buffer) *buf;
		if (!(buf = arr->_buf)) {
			return 0;
		}
		if (!(info = buf->_typeinfo)
		    || info->type != MPT_ENUM(TypeValFmt)) {
			return MPT_ERROR(BadType);
		}
		return buf->_used / sizeof(fmt);
	}
	/* get elements from iterator */
	it = 0;
	if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) > 0
	    && it
	    && (curr  = it->_vptr->get(it, MPT_ENUM(TypeValFmt), &fmt)) >= 0) {
		if (!curr) {
			mpt_array_clone(arr, 0);
			return 0;
		}
		ret = 0;
		do {
			if ((curr = it->_vptr->advance(it)) < 0) {
				break;
			}
			if (mpt_valfmt_add(&tmp, fmt) < 0) {
				mpt_array_clone(&tmp, 0);
				return MPT_ERROR(BadOperation);
			}
			if (!curr) {
				break;
			}
			++ret;
			curr = it->_vptr->get(it, MPT_ENUM(TypeValFmt), &fmt);
		}
		while (curr > 0);
	}
	/* get elements from value */
	else if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeValue), &val)) < 0) {
		const char *from = 0;
		if ((ret = src->_vptr->conv(src, 's', &from)) < 0) {
			return ret;
		}
		if (ret && (ret = mpt_valfmt_parse(&tmp, from)) < 0) {
			return ret;
		}
		ret = 0;
	}
	/* get elements from value elements */
	else if (val.fmt) {
		static const char valfmt[] = { MPT_ENUM(TypeValFmt), 0 };
		if (!val.ptr) {
			return MPT_ERROR(BadValue);
		}
		while ((curr = mpt_value_read(&val, valfmt, &fmt))) {
			if (curr < 0) {
				mpt_array_clone(&tmp, 0);
				return MPT_ERROR(BadType);
			}
			if (mpt_valfmt_add(&tmp, fmt) < 0) {
				mpt_array_clone(&tmp, 0);
				return MPT_ERROR(BadOperation);
			}
		}
		ret = 0;
	}
	/* get elements from string value */
	else if (val.ptr) {
		if ((ret = mpt_valfmt_parse(&tmp, val.ptr)) < 0) {
			return ret;
		}
		ret = 0;
	}
	mpt_array_clone(arr, &tmp);
	mpt_array_clone(&tmp, 0);
	return ret;
}
