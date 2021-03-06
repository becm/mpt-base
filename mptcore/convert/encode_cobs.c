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
# define MPT_COBS_MAXLEN 0xff
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
extern ssize_t mpt_encode_cobs(MPT_STRUCT(encode_state) *info, const struct iovec *cobs, const struct iovec *base)
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
	
	/* uninitialized target */
	if (!(dst = cobs->iov_base)) {
		if (code || len || left) {
			return MPT_ERROR(BadArgument);
		}
		return MPT_ERROR(MissingBuffer);
	}
	/* bad encoder state */
	if ((len > left)
	    || (code > (left -= len))
	    || (code > left)) {
		return MPT_ERROR(BadArgument);
	}
	dst += len;
	
	/* message termination */
	if (!base) {
		/* need enough data to save end */
		if (code >= left) {
			return MPT_ERROR(MissingBuffer);
		}
		/* special case: empty message */
		if (!code) {
			if (left < 2) {
				return MPT_ERROR(MissingBuffer);
			}
			/* '\x00\x00' not allowed in cobs data stream.
			 * Decoder consumes single encoded zero. */
			dst[0] = 1;
			dst[1] = 0;
			len = 2;
		}
		/* cobs message termination */
		else {
			*dst = code;
			dst[code++] = 0;
			len += code;
		}
		info->_ctx = 0;
		info->done = len;
		info->scratch = 0;
		
		return 0;
	}
	src = base->iov_base;
	len = base->iov_len;
	
	/* start priority data */
	if (!len) {
		return MPT_ERROR(BadValue);
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
				return MPT_ERROR(BadValue);
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
		if (!(left -= code)) return MPT_ERROR(MissingBuffer);
	} else {
		if (left <= (code = 1)) return MPT_ERROR(MissingBuffer);
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
