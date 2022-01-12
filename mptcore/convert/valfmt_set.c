/*!
 * MPT core library
 *   create value format data
 */

#include "meta.h"
#include "array.h"
#include "types.h"

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
extern int mpt_valfmt_set(MPT_STRUCT(array) *arr, MPT_INTERFACE(convertable) *src)
{
	MPT_INTERFACE(iterator) *it;
	MPT_STRUCT(array) tmp = MPT_ARRAY_INIT;
	MPT_STRUCT(value_format) fmt;
	int ret, curr;
	
	if (!src) {
		static const MPT_STRUCT(type_traits) *traits = 0;
		MPT_STRUCT(buffer) *buf;
		if (!(buf = arr->_buf)) {
			return 0;
		}
		/* initialize traits binding */
		if (!traits || (!(traits = mpt_type_traits(MPT_ENUM(TypeValFmt))))) {
			return MPT_ERROR(BadOperation);
		}
		if (!buf) {
			if (!(buf = _mpt_buffer_alloc(0, 0))) {
				return MPT_ERROR(BadOperation);
			}
			buf->_content_traits = traits;
		}
		else if (traits != buf->_content_traits) {
			return MPT_ERROR(BadType);
		}
		else {
			if ((buf = buf->_vptr->detach(buf, 0))) {
				return MPT_ERROR(BadOperation);
			}
			buf->_used = 0;
		}
		return MPT_ENUM(TypeValFmt);
	}
	/* get elements from iterator */
	it = 0;
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) > 0
	    && it) {
		ret = 0;
		while (1) {
			static const MPT_STRUCT(value_format) def = MPT_VALFMT_INIT;
			const MPT_STRUCT(value) *val;
			
			if (!(val = it->_vptr->value(it))) {
				break;
			}
			fmt = def;
			if (MPT_type_isConvertable(val->type)) {
				MPT_INTERFACE(convertable) *elem;
				if ((elem = *((void * const *) val->ptr))) {
					elem->_vptr->convert(elem, MPT_ENUM(TypeValFmt), &fmt);
				}
			}
			else if (val->type == MPT_ENUM(TypeValFmt) && val->ptr) {
				fmt = *((const MPT_STRUCT(value_format) *) val->ptr);
			}
			if ((curr = mpt_valfmt_add(&tmp, fmt) < 0)) {
				return curr;
			}
			if ((curr = it->_vptr->advance(it) <= 0)) {
				break;
			}
			++ret;
		}
	}
	/* assign single element */
	else if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeValFmt), &fmt)) >= 0) {
		if ((curr = mpt_valfmt_add(&tmp, fmt)) < 0) {
			mpt_array_clone(&tmp, 0);
			return curr;
		}
		ret = 0;
	}
	/* get elements from string value */
	else {
		const char *from = 0;
		if ((ret = src->_vptr->convert(src, 's', &from)) < 0) {
			return ret;
		}
		if (ret && (ret = mpt_valfmt_parse(&tmp, from)) < 0) {
			return ret;
		}
		ret = 0;
	}
	mpt_array_clone(arr, &tmp);
	mpt_array_clone(&tmp, 0);
	return ret;
}
