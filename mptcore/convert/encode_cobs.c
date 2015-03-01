/*!
 * COBS binary massage encoding
 */

#include "sys/uio.h"

#include "message.h"
#include "convert.h"

/* normal zero byte processing */
#ifndef MPT_cobs_zero
# define MPT_cobs_zero(c,d,r,s,l) (d[-c] = c, ++d, --r, 1)
#endif

#ifndef MPT_COBS_MAXLEN
# define MPT_COBS_MAXLEN 255


/*!
 * \ingroup mptConvert
 * \brief encode with COBS/R
 * 
 * Encode data with COBS and "tail inline" extension (COBS/R).
 * 
 * State is saved after finished COBS data.
 * 
 * Pass zero pointer @base to terminate message block.
 * 
 * \param info encoder state info
 * \param cobs buffer for COBS encoded data
 * \param base input data to append
 * 
 * \return length of processed data
 */
extern ssize_t mpt_encode_cobs_r(MPT_STRUCT(codestate) *info, const struct iovec *cobs, const struct iovec *base)
{
	uint8_t *dst, end;
	size_t off, left, code;
	
	if (!cobs || base || !(code = info->scratch)) {
		return mpt_encode_cobs(info, cobs, base);
	}
	off = info->done;
	left = cobs->iov_len;
	if ((off > left)
	    || !(dst = cobs->iov_base)) {
		return -1;
	}
	dst += off;
	/* tail inline condition */
	if (code > 1 && code < (end = dst[code-1])) {
		*dst = end;
		--code;
	}
	/* need enough data to save end */
	else if (left <= off) {
		return -2;
	}
	else {
		*dst = code;
	}
	/* cobs message termination */
	dst[code++] = 0;
	
	info->_ctx = 0;
	info->done = off + code;
	info->scratch = 0;
	
	return code;
}

/*!
 * \ingroup mptConvert
 * \brief encode COBS data
 * 
 * Encode data with regular COBS method.
 * 
 * State is saved after last finished COBS block.
 * Needs to be copied on encoded data relocation.
 * 
 * Pass zero pointer @base to terminate message block.
 * 
 * \param info encoder state info
 * \param cobs buffer for COBS encoded data
 * \param base input data to append
 * 
 * \return size of processed data
 */
extern ssize_t mpt_encode_cobs(MPT_STRUCT(codestate) *info, const struct iovec *cobs, const struct iovec *base)
#endif
{
	const uint8_t *src;
	uint8_t *dst, code;
	size_t left, len;
	
	if (!cobs) {
		info->_ctx = 0;
		info->done = 0;
		info->scratch = 0;
		return 0;
	}
	code = info->scratch;
	len  = info->done;
	left = cobs->iov_len;
	
	/* bad encoder state */
	if (!(dst = cobs->iov_base)
	    || (len > left)
	    || (code > (left -= len))
	    || (code > left)) {
		return -1;
	}
	dst += len;
	
	/* message termination */
	if (!base) {
		/* need enough data to save end */
		if (code > left) {
			return -2;
		}
		/* cobs message termination */
		*dst = code;
		dst[code++] = 0;
		len += code;
		
		info->_ctx = 0;
		info->done = len;
		info->scratch = 0;
		
		return code;
	}
	src = base->iov_base;
	len = base->iov_len;
	
	/* start priority data */
	if (!len) {
		return -1;
	}
	/* message deletion */
	if (!src) {
		struct iovec tmp;
		ssize_t pos = info->done;
		
		/* message in progress */
		if (info->_ctx) {
			--len;
		}
		tmp.iov_base = (void *) src;
		while (len--) {
			tmp.iov_len  = pos;
			if ((pos = mpt_memrchr(&tmp, 1, 0)) < 0) {
				return -1;
			}
		}
		info->_ctx = 0;
		info->done = pos;
		info->scratch = 0;
		
		/* save start of encoder state for next message */
		return pos;
	}
	
	/* remaining data in cobs */
	if (code) {
		if (!(left -= code)) return -1;
	} else {
		if (left <= (code = 1)) return -1;
		--left;
	}
	dst += code;
	
	while (1) {
		uint8_t curr;
		
		/* save COBS state */
		if (!len--) {
			dst[-code] = code;
			len = 0;
			break;
		}
		
		/* end of code block */
		if (!(curr = *src++)) {
			code = MPT_cobs_zero(code, dst, left, src, len);
		}
		else {
			*(dst++) = curr; --left;
			/* reached maximum length */
			if (++code == MPT_COBS_MAXLEN) {
				/* unable to save continuation state */
				if (!left) {
					--code; --dst;
					dst[-code] = code;
					++len; --src;
					break;
				}
				/* end of code block */
				dst[-code] = code;
				code = 1;
				dst++;
				left--;
			}
		}
		/* save current state */
		if (!left) {
			dst[-code] = code;
			break;
		}
	}
	
	/* update processed data */
	left = cobs->iov_len - left;
	info->_ctx += left - info->done - info->scratch;
	info->done = left - code;
	info->scratch = code;
	
	/* return consumed size */
	return base->iov_len - len;
}
