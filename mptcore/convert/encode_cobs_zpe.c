/*!
 * COBS/ZPE binary massage encoding
 */

#include "sys/uio.h"

#include "message.h"
#include "convert.h"

/* double zero elimination */
#define MPT_cobs_zero(c,d,r,s,l) (d[-c] = (\
	(l && (c > 1) && (c < 32) && !*s) \
	? (--l, ++s, c + MPT_COBS_MAXLEN) \
	: c), \
	1)
# define MPT_COBS_MAXLEN 224
/*!
 * \ingroup mptConvert
 * \brief encode with COBS/ZPE
 * 
 * Encode data with COBS method "zero pair elimination" (COBS/ZPE).
 * 
 * State is saved after finished COBS data.
 * 
 * Pass zero pointer @base to terminate message block.
 * 
 * \param info encoder state info
 * \param cobs buffer for COBS encoded data
 * \param base input data to append
 * 
 * \return size of processed data
 */
extern ssize_t mpt_encode_cobs_zpe(MPT_STRUCT(codestate) *info, const struct iovec *cobs, const struct iovec *base)
# include "encode_cobs.c"

/*!
 * \ingroup mptConvert
 * \brief encode with COBS/ZPE+R
 * 
 * Encode data with COBS extensions "zero pair elimination" and "tail inline" (COBS/ZPE+R).
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
extern ssize_t mpt_encode_cobs_zpe_r(MPT_STRUCT(codestate) *info, const struct iovec *cobs, const struct iovec *base)
{
	uint8_t *dst, end;
	size_t off, left, code;
	
	if (!cobs || base || !(code = info->scratch)) {
		return mpt_encode_cobs_zpe(info, cobs, base);
	}
	off = info->done + code;
	left  = cobs->iov_len;
	
	if ((off > left)
	    || !(dst = cobs->iov_base)) {
		return -1;
	}
	/* tail inline condition */
	if (code > 1 && code < (end = dst[off-1]) && end < MPT_COBS_MAXLEN) {
		*dst = end;
		--code;
		--off;
	}
	/* need enough data to save state of next message */
	else if (left <= off) {
		return -2;
	}
	/* cobs message termination */
	dst[off++] = 0;
	
	return code;
}
