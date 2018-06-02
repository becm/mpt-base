/*!
 * (v)printf operation to buffer data
 */

#include <stdio.h>
#include <errno.h>

#include <stdarg.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief print to array
 * 
 * Print formatted string to array data.
 * 
 * \param arr    array pointer
 * \param format fromat string
 * \param args   variable argument list
 * 
 * \return print result
 */
extern int mpt_vprintf(MPT_STRUCT(array) *arr, const char *format, va_list args)
{
	MPT_STRUCT(buffer) *buf;
	va_list tmp;
	int rval;
	char *base;
	size_t len, size, used;
	
	if (!(buf = arr->_buf)) {
		static const MPT_STRUCT(type_traits) info = MPT_TYPETRAIT_INIT(char, 'c');
		if (!(buf = _mpt_buffer_alloc(64, 0))) {
			return MPT_ERROR(BadOperation);
		}
		buf->_typeinfo = &info;
		arr->_buf = buf;
		size = buf->_size;
		used = 0;
		len = size;
	}
	/* require raw or character data buffer */
	else {
		const MPT_STRUCT(type_traits) *info;
		if (!(info = buf->_typeinfo) || info->type != 'c') {
			return MPT_ERROR(BadType);
		}
		size = buf->_size;
		used = buf->_used;
		len  = size - used;
		
		rval = len - len % 64;
		while ((size_t) rval < len) {
			rval += 64;
		}
		len = rval;
	}
	if (!(base = mpt_array_slice(arr, used, len))) {
		return MPT_ERROR(BadOperation);
	}
	buf = arr->_buf;
	
	va_copy(tmp, args);
	rval = vsnprintf(base, len, format, tmp);
	va_end(tmp);
	
	/* ignore broken implementations */
	if (rval < 0) {
		buf->_used = used;
		return MPT_ERROR(BadValue);
	}
	if (rval >= 0 && (size_t) rval <= len) {
		if ((size_t) rval < len) {
			base[rval] = '\0';
		}
		buf->_used = used + rval;
		return rval;
	}
	while (len <= (size_t) rval) {
		len += 64;
	}
	if (!(base = mpt_array_slice(arr, used, len))) {
		return MPT_ERROR(BadOperation);
	}
	size = used + len;
	if ((rval = vsnprintf(base, len, format, args)) > 0) {
		used += rval;
		if (used < size) {
			base[rval] = '\0';
			return rval;
		}
	}
	return MPT_ERROR(BadValue);
}

/*!
 * \ingroup mptArray
 * \brief print to array
 * 
 * Print formatted string to array data.
 * 
 * \param arr	array pointer
 * \param fmt	fromat string
 * 
 * \return print result
 */
extern int mpt_printf(MPT_STRUCT(array) *arr, const char *fmt, ... )
{
	va_list	va;
	int	ret;
	
	va_start(va, fmt);
	ret = mpt_vprintf(arr, fmt, va);
	va_end(va);
	
	return ret;
}
