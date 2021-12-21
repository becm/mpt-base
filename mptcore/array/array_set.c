/*!
 * MPT core library
 *   set typed data on array
 */

#include <limits.h>
#include <errno.h>

#include "types.h"

#include "array.h"

#ifndef SSIZE_MAX
# define SSIZE_MAX (SIZE_MAX/2)
#endif

/*!
 * \ingroup mptCore
 * \brief set typed data on array
 * 
 * Set data on array.
 * Checks for compatibility of data and array content.
 * 
 * \param arr target array
 * \param traits  type traits of data
 * \param len     raw size of new data
 * \param data    base address of new data
 * \param off     offset of new data, negative is relative to existing data end
 * \return location of assigned data
 */
extern void *mpt_array_set(MPT_STRUCT(array) *arr, const MPT_STRUCT(type_traits) *traits, size_t len, const void *data, long off)
{
	MPT_STRUCT(buffer) *buf;
	ssize_t pos;
	size_t size, total;
	
	if (!traits) {
		errno = EINVAL;
		return 0;
	}
	/* avoid incomplete element assignments */
	if (!(size = traits->size)
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
		if (off < 0) {
			pos += buf->_used;
		}
	}
	/* offset is invalid */
	if (pos < 0 || ((SIZE_MAX - pos) < len)) {
		errno = EINVAL;
		return 0;
	}
	
	total = pos + len;
	
	if (!buf) {
		buf = _mpt_buffer_alloc(total, 0);
		buf->_content_traits = traits;
		arr->_buf = buf;
	}
	else {
		int flags = buf->_vptr->get_flags(buf);
		
		if ((buf->_size < total)
		 || (MPT_ENUM(BufferImmutable) & flags)
		 || (MPT_ENUM(BufferShared) & flags)) {
			if (!(buf = buf->_vptr->detach(buf, total))) {
				return 0;
			}
			arr->_buf = buf;
		}
	}
	if ((off = mpt_buffer_set(buf, traits, pos, data, len)) < 0) {
		return 0;
	}
	return ((uint8_t *) (buf + 1)) + pos;
}
