/*!
 * COBS binary massage decoding
 */

#include <string.h>

#include "convert.h"

#ifndef MPT_COBS_MAXLEN

# define MPT_COBS_MAXLEN       224
# define MPT_cobs_reduce(c)    (((c) <= MPT_COBS_MAXLEN) ? (c) : (c) - MPT_COBS_MAXLEN + 1)
# define MPT_cobs_zadd(l,d,c)  if ((c) > MPT_COBS_MAXLEN) { *(d)++ = 0; *(d)++ = 0; (l)--; }
/*!
 *
 * \ingroup mptConvert
 * \brief compressed COBS decoder
 * 
 * Parts with length >224 encode data with two following zeros.
 * Detect and handle tail-inlined byte.
 * 
 * On optimal compression condition, decoded data can
 * require 1.5 times of coded message.
 * 
 * \param cobs  start of cobs data
 * \param clen  length of cobs data
 * \param dec   target data
 * \param dlen  length of target data
 * 
 * \retval 0         regular message end
 * \retval 1..255    destination buffer depleted, need at least more ...
 * \retval -255..-1  missing data to complete part
 * \retval -256      tail inlined zero detected
 */
extern int mpt_cobs_dec_zpe(const void *cobs, size_t *clen, void *dec, size_t *dlen)
# include "cobs_dec.c"


# undef MPT_COBS_MAXLEN
# undef MPT_cobs_reduce
# undef MPT_cobs_zadd
# define MPT_COBS_MAXLEN       255
# define MPT_cobs_reduce(c)    (c)
# define MPT_cobs_zadd(l,d,c)
/*!
 * \ingroup mptConvert
 * \brief COBS decoder
 * 
 * In-place operation is possible with (dec <= cobs) address.
 * Detect and handle tail-inlined byte.
 * 
 * \param cobs  start of cobs data
 * \param clen  length of cobs data
 * \param dec   target data
 * \param dlen  length of target data
 * 
 * \retval 0         regular message end
 * \retval 1..255    destination buffer depleted, need at least more ...
 * \retval -255..-1  missing data to complete part
 * \retval -256      tail inlined zero detected
 */
extern int mpt_cobs_dec(const void *cobs, size_t *clen, void *dec, size_t *dlen)
# include "cobs_dec.c"
#else
{
	const uint8_t *src = cobs;
	uint8_t *dst = dec, old = 0;
	ssize_t len = *clen, left = *dlen;
	
	if (!cobs) {
		return MPT_COBS_MAXLEN;
	}
	while (len--) {
		uint8_t curr, code, i;
		
		/* start of new block */
		if (!(code = *src++)) {
			--*clen;
			return 0;
		}
		/* previous part end */
		if (old) {
			*dlen = --left;
			*dst++ = 0;
		}
		old = code;
		
		curr = MPT_cobs_reduce(code);
		
		/* indicate missing input size */
		if (curr > len) {
			const uint8_t *in;
			/* tail inline in remaining data */
			if ((in = memchr(src, 0, len))) {
				if ((left = (in-src))) {
					memmove(dst, src, left);
				}
				dst[left] = code;
				*clen -= left + 2;
				*dlen -= left + 1;
				return -256;
			}
			return -curr;
		}
		/* indicate missing buffer */
		if (curr > left) {
			return curr;
		}
		for (i = 1; i < curr; i++) {
			--len; --left;
			/* save data */
			if ((*dst++ = *src++)) {
				continue;
			}
			/* tail byte inline */
			dst[-1] = code;
			*clen = len;
			*dlen = left;
			return -256;
		}
		/* skip regular trailing block zero */
		if (code >= MPT_COBS_MAXLEN) {
			MPT_cobs_zadd(left, dst, curr)
			old = 0;
		}
		/* no remaining space for decoded data */
		else if (!left) {
			*dlen = 0;
			/* next element is message end */
			if (len && !*src) {
				*clen = len - 1;
				return 0;
			}
			*clen = len;
			return len ? 1 : -1;
		}
		*clen = len;
		*dlen = left;
	}
	return -1;
}
#endif
