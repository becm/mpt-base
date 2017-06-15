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
		if (!(buf = _mpt_buffer_alloc(64))) {
			return MPT_ERROR(BadOperation);
		}
	}
	/* require raw or character data buffer */
	else {
		int type = buf->_vptr->content(buf);
		if (type && type != 'c') {
			return MPT_ERROR(BadType);
		}
	}
	size = buf->_size;
	used = buf->_used;
	
	if ((len = size - used) < 64) {
		if (!(buf = buf->_vptr->detach(buf, size + 64))) {
			return MPT_ERROR(BadOperation);
		}
		len = (size = buf->_size) - used;
		arr->_buf = buf;
	}
	base = (char *) (buf + 1);
	
	va_copy(tmp, args);
	rval = vsnprintf(base+used, len, format, tmp);
	va_end(tmp);
	
	if (rval >= 0 && (size_t) rval <= len) {
		if ((used = (buf->_used += rval)) < size)
			base[used] = '\0';
		return rval;
	}
	/* ignore broken implementations */
	else if (rval < 0) {
		return MPT_ERROR(BadValue);
	}
	else if (!(buf = buf->_vptr->detach(buf, used + rval + 1))) {
		return MPT_ERROR(BadOperation);
	}
	base = (char *) (buf + 1);
	len  = buf->_size - buf->_used;
	if ((rval = vsnprintf(base+used, len, format, args)) > 0) {
		buf->_used += rval;
	}
	return rval < 0 ? MPT_ERROR(BadValue) : rval;
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
