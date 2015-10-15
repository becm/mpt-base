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
 * \param arr	array pointer
 * \param format fromat string
 * \param args	variable argument list
 * 
 * \return print result
 */
extern int mpt_vprintf(MPT_STRUCT(array) *arr, const char *format, va_list args)
{
	MPT_STRUCT(buffer) *buf, *(*resize)(MPT_STRUCT(buffer) *, size_t );
	va_list tmp;
	int	rval;
	char	*base;
	size_t	len, size, used;
	
	if (!(buf = arr->_buf)) {
		used = size = 0;
		base = 0;
		resize = _mpt_buffer_realloc;
	} else {
		size = buf->size;
		used = buf->used;
		base = (char *) (buf+1);
		resize = buf->resize;
	}
	
	if ((len = size - used) < 64) {
		if (!base) {
			if (!resize) {
				return -1;
			}
		}
		if (resize && (buf = resize(buf, size + 64))) {
			len = (size = buf->size) - used;
			arr->_buf = buf;
			base = (char *) (buf+1);
		}
	}
	if (!buf) {
		return -1;
	}
	va_copy(tmp, args);
	rval = vsnprintf(base+used, len, format, tmp);
	va_end(tmp);
	
	if (rval >= 0 && (size_t) rval <= len) {
		if ((used = (buf->used += rval)) < size)
			base[used] = '\0';
		return rval;
	}
	else if (!resize) {
		errno = ERANGE;
		return -1;
	}
	else if (rval < 0) {
		if (!(buf = resize(buf, size + 128))) {
			return -2;
		}
		return mpt_vprintf(arr, format, args);
	}
	else if (!(buf = resize(buf, used + rval + 1))) {
		return -1;
	} else {
		base = (char *) (buf+1);
		len  = buf->size - buf->used;
		if ((rval = vsnprintf(base+used, len, format, args)) > 0) {
			buf->used += rval;
		}
		return rval;
	}
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
