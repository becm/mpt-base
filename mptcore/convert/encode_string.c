
#include <string.h>
#include <sys/uio.h>

#include "message.h"
#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief zero-terminated data
 * 
 * Check input for illegal inline zero
 * and copy to destination
 * 
 * \param info  coding information
 * \param to    target data for encoding
 * \param from  data to encode
 * 
 * \return encoded data size
 */
extern ssize_t mpt_encode_string(MPT_STRUCT(encode_state) *info, const struct iovec *to, const struct iovec *from)
{
	uint8_t *base;
	size_t sep, len, off, max;
	
	if (!to) {
		info->_ctx = 0;
		info->done = 0;
		info->scratch = 0;
		return 0;
	}
	sep = info->scratch;
	off = info->done + info->scratch;
	
	base = to->iov_base;
	max  = to->iov_len;
	
	if (off > max) {
		return -1;
	}
	/* terminate string */
	if (!from) {
		max -= off;
		
		if (!sep) {
			if (!max) {
				return MPT_ERROR(MissingBuffer);
			}
			base[off++] = info->_ctx;
		}
		else if (max < sep) {
			return MPT_ERROR(MissingBuffer);
		}
		else {
			base += off;
			memcpy(base-sep, base, sep);
			off += sep;
		}
		info->done = off;
		
		/* push delimiter data */
		return 0;
	}
	/* start priority data */
	if (!(len = from->iov_len)) {
		return MPT_ERROR(BadArgument);
	}
	/* delete buffered data */
	if (!from->iov_base) {
		/* no data available */
		if (!off) {
			return MPT_ERROR(MissingData);
		}
		/* no active message, remove endbyte */
		if (!(base)[off-1]) {
			--off;
		}
		while (len--) {
			struct iovec tmp;
			ssize_t pos;
			
			tmp.iov_base = base;
			tmp.iov_len  = off;
			
			/* find message separator */
			if ((pos = mpt_memrchr(&tmp, 1, info->_ctx)) >= 0) {
				off = pos;
				continue;
			}
			return MPT_ERROR(MissingData);
		}
		info->done -= off;
		
		return off;
	}
	if (!(max -= off)) {
		return MPT_ERROR(MissingBuffer);
	}
	base += off;
	if (len < max) {
		max = from->iov_len;
	}
	/* invalid data */
	if (len) {
		if (!sep) {
			if (max && memchr(from->iov_base, info->_ctx, max)) {
				return MPT_ERROR(BadEncoding);
			}
		}
		/* check longer separator pattern */
		else {
			const uint8_t *cmp = base, *val;
			ssize_t i, total;
			
			/* relative compare start position */
			i = (off < len) ? -off : 1 - len;
			
			total = max - len;
			for (val = base - len + i; i < total; ++i, ++val) {
				const uint8_t *v2, *c2;
				ssize_t j, k;
				if (!i) val = from->iov_base;
				if (*val != *cmp) continue;
				v2 = val + 1;
				c2 = cmp + 1;
				for (j = i+1, k = j+len; i < k; ++j) {
					if (!j) v2 = from->iov_base;
					if (*(v2++) != *(c2++)) {
						break;
					}
				}
				if (j == k) {
					return -3;
				}
			}
			/* move delimiter string */
			memmove(base+max-sep, base-sep, sep);
		}
	}
	else if (memchr(from->iov_base, info->_ctx, max)) {
		return MPT_ERROR(BadEncoding);
	}
	/* copy source data */
	memcpy(base-sep, from->iov_base, max);
	off += max;
	
	info->done = off;
	
	return max;
}
