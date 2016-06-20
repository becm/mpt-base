
#include "sys/uio.h"

#include "convert.h"

#ifndef MPT_cobs_check_inline
# define MPT_cobs_check_inline(c,e,p)    ((c) < ((e) = (p)[(c)-1]))
#endif

#ifndef MPT_encode_cobs_regular
# define MPT_encode_cobs_regular(i,c,d)  mpt_encode_cobs(i, c, d)
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
#endif
{
	uint8_t *dst, end;
	size_t off, left, code;
	
	if (!cobs || base || !(code = info->scratch)) {
		return MPT_encode_cobs_regular(info, cobs, base);
	}
	off = info->done;
	left = cobs->iov_len;
	if ((off > left)
	    || !(dst = cobs->iov_base)) {
		return -1;
	}
	dst += off;
	/* tail inline condition */
	if (code > 1 && MPT_cobs_check_inline(code, end, dst)) {
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
	
	return 0;
}
