/*!
 * slice data write
 */

#include <string.h>

#include "types.h"
#include "output.h"

#include "array.h"

static ssize_t _fast_append(MPT_STRUCT(slice) *sl, size_t nblk, const void *from, size_t esze)
{
	MPT_STRUCT(buffer) *buf = sl->_a._buf;
	size_t pos, avail, add, take, used;
	uint8_t *ptr;
	pos = sl->_off + sl->_len;
	avail = buf->_size - pos;
	
	add = esze;
	take = 0;
	while (add < avail && nblk--) {
		take += esze;
	}
	ptr = (void *) (buf + 1);
	if (from) {
		memcpy(ptr + pos, from, take);
	} else {
		memset(ptr + pos, 0, take);
	}
	sl->_len += take;
	pos += take;
	used = buf->_used;
	if (used > pos) {
		buf->_used = used;
	}
	return take;
}

/*!
 * \ingroup mptArray
 * \brief write to slice
 * 
 * Append data blocks to slice.
 * 
 * \param sl    array descriptor
 * \param nblk  number of data blocks
 * \param from  data base address
 * \param size  data element size
 * 
 * \return number of elements written
 */
extern ssize_t mpt_slice_write(MPT_STRUCT(slice) *sl, size_t nblk, const void *from, size_t size)
{
	MPT_STRUCT(buffer) *buf, *next;
	uint8_t *ptr;
	size_t used, avail, pos;
	
	pos = sl->_off + sl->_len;
	used = 0;
	avail = 0;
	
	if ((buf = sl->_a._buf)) {
		if (buf->_content_traits) {
			return MPT_ERROR(BadType);
		}
		used = buf->_used;
		avail = buf->_size;
	}
	/* fix slice area */
	if (pos > used) {
		mpt_log(0, __func__, MPT_LOG(Critical), "%s: (len = %zi) > (data = %zi)",
		        MPT_tr("inconsistent slice state"), pos, used);
		if (sl->_off >= used) {
			sl->_off = used;
			sl->_len = 0;
		} else {
			sl->_len = used - sl->_off;
		}
		pos = used;
	}
	avail -= pos;
	/* prepare memory */
	if (!size) {
		if (!nblk) {
			return 0;
		}
		if (from) {
			return MPT_ERROR(BadArgument);
		}
		if (nblk <= avail) {
			return avail / nblk;
		}
		avail += nblk;
		if (!mpt_array_slice(&sl->_a, pos, nblk)) {
			return -1;
		}
		mpt_buffer_cut(sl->_a._buf, pos, nblk);
		return avail / nblk;
	}
	/* fast data append path */
	if (buf
	 && !((buf->_vptr->get_flags(buf)) & (MPT_ENUM(BufferImmutable) | MPT_ENUM(BufferShared)))) {
		size_t off;
		if (!nblk) {
			return 0;
		}
		/* simple data copy */
		if (avail >= size) {
			return _fast_append(sl, nblk, from, size);
		}
		/* move data to buffer front */
		if ((off = sl->_off) && (avail + off) >= size) {
			ptr = (void *) (buf + 1);
			if ((used = sl->_len)) {
				memmove(ptr, ptr + off, used);
			}
			buf->_used = used;
			sl->_off = 0;
			return _fast_append(sl, nblk, from, size);
		}
	}
	/* get space for needed data size */
	while (!(next = _mpt_buffer_alloc(sl->_len + nblk * size, 0))) {
		if (!(nblk /= 2)) {
			return MPT_ERROR(BadOperation);
		}
	}
	ptr = (void *) (next + 1);
	pos = sl->_len;
	if (buf && pos) {
		uint8_t *old = (void *) (buf + 1);
		memcpy(ptr, old + sl->_off, pos);
	}
	ptr += pos;
	if (nblk) {
		size_t total = nblk * size;
		if (from) {
			memcpy(ptr, from, total);
		} else {
			memset(ptr, 0, total);
		}
	}
	if (buf) {
		buf->_vptr->unref(buf);
	}
	return nblk;
}
