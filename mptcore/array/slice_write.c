/*!
 * slice data write
 */

#include <string.h>

#include "array.h"
/*!
 * \ingroup mptArray
 * \brief write to slice
 * 
 * Append data blocks to slice.
 * 
 * \param sl    array descriptor
 * \param nblk  number of data blocks
 * \param from  data base address
 * \param len   data element size
 * 
 * \return number of elements written
 */
extern ssize_t mpt_slice_write(MPT_STRUCT(slice) *sl, size_t nblk, const void *from, size_t esze)
{
	MPT_STRUCT(buffer) *buf;
	size_t used, avail, pos, total;
	
	if ((buf = sl->_a._buf)) {
		used = buf->used;
		avail = buf->size - used;
	}
	else {
		used = 0;
		avail = 0;
	}
	pos = sl->_off + sl->_len;
	
	/* fix slice area */
	if (pos > used) {
		if (sl->_off >= used) {
			sl->_off = 0;
			sl->_len = 0;
		} else {
			sl->_len = used - sl->_off;
		}
	}
	/* prepare memory */
	if (!esze) {
		if (!nblk) return 0;
		if (nblk <= avail) {
			return avail/nblk;
		}
		avail += nblk;
		if (!mpt_array_slice(&sl->_a, pos, nblk)) {
			return -1;
		}
		mpt_array_cut(&sl->_a, pos, nblk);
		return avail/nblk;
	}
	total = nblk * esze;
	
	/* fast data append path */
	if (total < avail && !buf->shared) {
		if (mpt_array_append(&sl->_a, total, from)) {
			sl->_len = pos + total;
			return nblk;
		}
	}
	/* remove offset data */
	if (sl->_off) {
		uint8_t *to = (uint8_t *) (buf+1);
		
		if (!(to = mpt_array_slice(&sl->_a, 0, pos + esze))) {
			return -1;
		}
		buf = sl->_a._buf;
		memmove(to, to+sl->_off, pos = sl->_len);
		sl->_off = 0;
		buf->used = pos;
		avail = buf->size - pos;
	}
	/* fast data append */
	if (total <= avail && mpt_array_append(&sl->_a, total, from)) {
		sl->_len = sl->_a._buf->used;
		return nblk;
	}
        if ((avail /= esze)) {
		size_t add = avail*esze;
		if (mpt_array_append(&sl->_a, add, from)) {
			nblk -= avail;
			from = ((uint8_t *) from) + add;
		} else {
			avail = 0;
		}
	}
	while (nblk-- && mpt_array_append(&sl->_a, esze, from)) {
		++avail;
		from = ((uint8_t *) from) + esze;
	}
	sl->_len = sl->_a._buf->used;
	return avail ? (ssize_t) avail : -2;
}
