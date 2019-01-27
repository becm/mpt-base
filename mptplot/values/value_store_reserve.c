/*!
 * MPT plot library
 *    reserve data on value storage
 */

#include <string.h>

#include "types.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief reserve value space
 * 
 * Get or register input reference type.
 * 
 * \return input id
 */
extern void *mpt_value_store_reserve(MPT_STRUCT(array) *arr, int type, size_t len, long off)
{
	MPT_STRUCT(buffer) *buf;
	size_t pos;
	
	if (type < 0) {
		return 0;
	}
	if ((buf = arr->_buf)) {
		const MPT_STRUCT(type_traits) *info;
		ssize_t size;
		if (!(info = buf->_typeinfo)) {
			return 0;
		}
		if (type != info->type
		    || !(size = info->size)) {
			return 0;
		}
		/* avoid incomplete element assignments */
		if (len % size) {
			return 0;
		}
		size *= off;
		/* relative to data end */
		if (size < 0 && (size += buf->_used) < 0) {
			return 0;
		}
		return mpt_array_slice(arr, size, len);
	}
	else {
		MPT_STRUCT(type_traits) info;
		ssize_t size;
		memset(&info, 0, sizeof(info));
		if ((size = mpt_valsize(type)) < 0) {
			return 0;
		}
		// no existing data
		if (off < 0) {
			return 0;
		}
		if (len % size) {
			return 0;
		}
		pos = off * size;
		len += pos;
		
		info.type = type;
		info.size = size;
		info.base = type;
		if (!(buf = _mpt_buffer_alloc(len, &info))) {
		    return 0;
		}
		arr->_buf = buf;
		buf->_used = len;
		if (pos) {
			uint8_t *data = memset(buf + 1, 0, pos);
			return data + pos;
		}
		return buf + 1;
    }
}
