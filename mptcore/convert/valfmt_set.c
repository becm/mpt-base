/*!
 * set history state
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "array.h"
#include "meta.h"

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
extern int mpt_valfmt_set(MPT_STRUCT(array) *arr, MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(array) tmp = MPT_ARRAY_INIT;
	MPT_STRUCT(valfmt) fmt;
	char *from;
	int len;
	
	if (!src) {
		return arr->_buf ? arr->_buf->used / sizeof(MPT_STRUCT(valfmt)) : 0;
	}
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeValFmt) | MPT_ENUM(ValueConsume), &fmt)) >= 0) {
		int curr;
		if (!mpt_array_append(&tmp, sizeof(fmt), &fmt)) {
			return MPT_ERROR(BadOperation);
		}
		while ((curr = src->_vptr->conv(src, MPT_ENUM(TypeValFmt) | MPT_ENUM(ValueConsume), &fmt)) >= 0) {
			if (!mpt_array_append(&tmp, sizeof(fmt), &fmt)) {
				mpt_array_clone(&tmp, 0);
				return MPT_ERROR(BadOperation);
			}
			len += curr;
		}
		mpt_array_clone(arr, &tmp);
		mpt_array_clone(&tmp, 0);
		return len;
	}
	if ((len = src->_vptr->conv(src, 's', &from)) < 0) {
		return len;
	}
	else if (!from) {
		mpt_array_clone(arr, 0);
	}
	else {
		if ((len = mpt_valfmt_parse(&tmp, from)) >= 0) {
			mpt_array_clone(arr, &tmp);
		}
		mpt_array_clone(&tmp, 0);
	}
	return len;
}
