/*!
 * MPT plot library
 *    reserve data on value storage
 */

#include <limits.h>
#include <string.h>
#include <errno.h>

#include "types.h"

#include "values.h"

#ifndef SSIZE_MAX
# define SSIZE_MAX (SIZE_MAX/2)
#endif

/*!
 * \ingroup mptValues
 * \brief reserve value space
 * 
 * Get or register input reference type.
 * 
 * \return input id
 */
extern void *mpt_value_store_reserve(MPT_STRUCT(array) *arr, const MPT_STRUCT(type_traits) *traits, size_t len, long off)
{
	MPT_STRUCT(buffer) *buf;
	ssize_t pos;
	size_t size;
	
	if (!traits) {
		return 0;
	}
	/* avoid incomplete element assignments */
	if (!(size = traits->size)
	 || ((SSIZE_MAX / ((ssize_t) size)) < off)
	 || (len % size)) {
		errno = EINVAL;
		return 0;
	}
	pos = off * size;
	
	if ((buf = arr->_buf)) {
		if (traits != buf->_content_traits) {
			errno = EINVAL;
			return 0;
		}
		/* relative to data end */
		if (pos < 0 && (pos += buf->_used) < 0) {
			errno = EINVAL;
			return 0;
		}
	}
	/* no existing data without buffer */
	else if (off < 0) {
		errno = EINVAL;
		return 0;
	}
	
	if (buf) {
		return mpt_array_slice(arr, pos, len);
	}
	
	if (!(buf = _mpt_buffer_alloc(pos + len, 0))) {
		return 0;
	}
	buf->_content_traits = traits;
	
	if (!pos) {
		buf->_used = len;
		arr->_buf = buf;
		return buf + 1;
	}
	else {
		uint8_t *data = (void *) (buf + 1);
		int (*init)(void *, const void *);
		
		if (!(init = traits->init)) {
			memset(data, 0, pos);
		}
		else {
			ssize_t curr;
			for (curr = 0; curr < pos; curr += size) {
				if (init(data + curr,  0) < 0) {
					void (*fini)(void *) = traits->fini;
					if (fini) {
						for (pos = 0; pos < curr; pos += size) {
							fini(data + pos);
						}
					}
					buf->_vptr->unref(buf);
					errno = ENOTSUP;
					return 0;
				}
			}
		}
		arr->_buf = buf;
		return data + pos;
	}
}
