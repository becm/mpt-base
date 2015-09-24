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
	: c), ++d, --r, \
	1)
#define MPT_COBS_MAXLEN 0xdf /* = 223 */
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
#include "encode_cobs.c"

#define MPT_encode_cobs_regular(i,c,d)  mpt_encode_cobs_zpe(i,c,d)
#define MPT_cobs_check_inline(c,e,p) \
	(((c) <= MPT_COBS_MAXLEN) && \
	((c) < ((e) = (p)[(c)-1])) && \
	((e) <= MPT_COBS_MAXLEN))
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
#include "encode_cobs_r.c"
