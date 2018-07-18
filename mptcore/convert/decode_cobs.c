#include <string.h>

#include <sys/uio.h>

#include <arpa/inet.h>

#include "message.h"
#include "convert.h"

#ifndef MPT_cobs_state
# define MPT_cobs_state(code, pos) ((pos) * 0x100 + (code))
#endif

#ifndef MPT_cobs_len_data
# define MPT_cobs_len_data(c)   ((c) - 1)
#endif

#ifndef MPT_cobs_len_zero
# define MPT_cobs_len_zero(c,n) ((((c) < MPT_COBS_MAXLEN) && n) ? 1 : 0)
#endif

#ifndef MPT_cobs_max_dec
# define MPT_cobs_max_dec(c)    ((c) - ((c) / MPT_COBS_MAXLEN))
#endif

#if __STDC_VERSION__ < 199901L
# define restrict __restrict__
#endif

#ifndef MPT_COBS_MAXLEN
/*!
 * \ingroup mptConvert
 * \brief decode COBS data
 * 
 * Decode data in COBS format in vector array.
 * 
 * New message data begins at \dec.data.off ,
 * encoded data continues at \dec.pos .
 * 
 * Pass \sourcelen = 0 to get current message length
 * on single data element.
 * No data change is performed in this case.
 * 
 * Pass \source = 0 to get maximum required message size
 * for remaining data length.
 * No data change is performed in this case.
 * 
 * Pass \sourcelen = 0 and \source = 0 to reset
 * \decode_state data.
 * 
 * \param dec       metadata for decoder
 * \param source    data elements
 * \param sourcelen length of data array
 * 
 * \return size of message
 */
extern int mpt_decode_cobs
#define MPT_COBS_MAXLEN 255
#else
static int _decode
#endif
(MPT_STRUCT(decode_state) *dec, const struct iovec *source, size_t sourcelen)
{
	MPT_STRUCT(message) tmp;
	const struct iovec *dvec;
	const uint8_t *restrict src;
	uint8_t *restrict dst;
	size_t proc, mlen, done, dlen, clen;
	uint8_t code, pos;
	
	/* used message buffer */
	mlen = dec->data.len;
	
	if (!source) {
		if (!sourcelen) {
			dec->_ctx = 0;
		} else {
			/* get expected/sufficient scratch area */
			return MPT_cobs_max_dec(sourcelen) + mlen;
		}
		return 0;
	}
	/* decoder data offsets */
	code = dec->_ctx & 0xff;
	done = dec->data.pos;
	proc = dec->curr;
	
	tmp.base = 0;
	tmp.used = 0;
	tmp.cont = (void *) source;
	tmp.clen = sourcelen ? sourcelen : 1;
	
	/* consume processed data */
	dlen = done + mlen;
	if ((dlen > proc)
	    || (mpt_message_read(&tmp, dlen, 0) < dlen)) {
		return MPT_ERROR(BadArgument);
	}
	/* space available for target data */
	proc -= dlen;
	
	/* consume previous message */
	if (dec->data.msg >= 0) {
		if (!sourcelen) {
			return MPT_ERROR(BadOperation);
		}
		clen = dec->data.msg;
		done += clen;
		mlen -= clen;
		dec->data.len = mlen;
		dec->data.msg = -1;
	}
	/* align offset for target data */
	if (!mlen) {
		const uint8_t *addr;
		size_t curr, align = 0x10;
		
		if (!sourcelen) {
			return MPT_ERROR(BadOperation);
		}
		/* set current message done */
		done = dlen;
		
		addr = tmp.base;
		curr = tmp.used;
		
		/* target base alignment */
		while (align > 1) {
			size_t post = 0;
			if ((curr < align/2)
			    || (proc < (post = ((uintptr_t) addr) & (align - 1)))) {
				align /= 2;
				continue;
			}
			done += post;
			proc -= post;
			mpt_message_read(&tmp, post, 0);
			break;
		}
		dec->data.pos = done;
	}
	/* decoded data start */
	dst  = (uint8_t *) tmp.base;
	dlen = tmp.used;
	dvec = tmp.cont;
	
	/* encoded data start */
	if (mpt_message_read(&tmp, proc, 0) < proc) {
		return MPT_ERROR(BadArgument);
	}
	src  = tmp.base;
	clen = tmp.used;
	
	/* finished with complete block/message */
	if (!code) {
		if (!mpt_message_read(&tmp, 1, &code)) {
			return 0;
		}
		src  = tmp.base;
		clen = tmp.used;
		++proc;
		
		/* double/leading zero */
		if (!code) {
			dec->curr = proc;
			return MPT_ERROR(BadValue);
		}
	}
	/* position in last save operation */
	pos = (dec->_ctx & 0xff00) / 0x100;
	
	while (1) {
		uint8_t curr, next;
		
		/* message finished */
		if (!code) {
			dec->_ctx = 0;
			dec->data.pos = done;
			dec->data.len = mlen;
			dec->data.msg = mlen;
			dec->curr = done + mlen + proc;
			return 1;
		}
		/* length of non-zero part */
		curr = MPT_cobs_len_data(code);
		
		for (; pos < curr; ++pos) {
			uint8_t val;
			while (!clen--) {
				if (!tmp.clen--) {
					dec->_ctx = MPT_cobs_state(code, pos);
					dec->curr = done + mlen + proc;
					dec->data.len = mlen;
					return 0;
				}
				src  = tmp.cont->iov_base;
				clen = tmp.cont->iov_len;
				++tmp.cont;
			}
			/* inline zero byte */
			if (!(val = *src++)) {
				dec->_ctx = MPT_cobs_state(code, pos);
				dec->curr = done + mlen + proc;
				dec->data.len = mlen;
				return MPT_ERROR(MissingData);
			}
			/* no remaining target space */
			if (!proc) {
				dec->_ctx = MPT_cobs_state(code, pos);
				dec->curr = done + mlen + proc;
				dec->data.len = mlen;
				return 0;
			}
			/* no remaining data on current part */
			while (!dlen--) {
				dst  = (uint8_t *) dvec->iov_base;
				dlen = dvec->iov_len;
				++dvec; /* total position limited by `proc` and dst <= src */
			}
			*dst++ = val;
			++mlen;
		}
		/* only process first block */
		if (!sourcelen) {
			dec->_ctx = MPT_cobs_state(code, pos);
			dec->curr = done + mlen + proc;
			dec->data.len = mlen;
			return 0;
		}
		/* read next element */
		while (!clen--) {
			if (!tmp.clen--) {
				dec->_ctx = MPT_cobs_state(code, pos);
				dec->curr = done + mlen + proc;
				dec->data.len = mlen;
				return 0;
			}
			src  = tmp.cont->iov_base;
			clen = tmp.cont->iov_len;
			++tmp.cont;
		}
		next = *src++;
		
		/* needed numbers of zeros */
		curr += MPT_cobs_len_zero(code, next);
		
		for (; pos < curr; ++pos) {
			/* no remaining target space */
			if (!proc) {
				dec->_ctx = MPT_cobs_state(code, pos);
				dec->curr = done + mlen + proc;
				dec->data.len = mlen;
				return MPT_ERROR(MissingBuffer);
			}
			/* no remaining data on current part */
			while (!dlen--) {
				dst  = (uint8_t *) dvec->iov_base;
				dlen = dvec->iov_len;
				++dvec; /* total position limited by `proc` and dst <= src */
			}
			*dst++ = 0;
			++mlen;
			--proc;
		}
		/* save next part code */
		++proc;
		code = next;
		pos = 0;
	}
}

#ifndef MPT_cobs_dec_regular
/*!
 * \ingroup mptConvert
 * \brief decode COBS/R data
 * 
 * Decode data in COBS format in vector array.
 * 
 * New message data begins at \info.content.pos ,
 * encoded data continues at \info.work.pos .
 * 
 * Pass \sourcelen = 0 to get current message length
 * on single data element.
 * No data change is performed in this case.
 * 
 * Pass \source = 0 to get maximum required message size
 * for remaining data length.
 * No data change is performed in this case.
 * 
 * Pass \sourcelen = 0 and \source = 0 to reset
 * \decode_state data.
 * 
 * \param dec       metadata for decoder
 * \param source    data elements
 * \param sourcelen length of data array
 * 
 * \return size of message
 */
extern int mpt_decode_cobs_r
# define MPT_cobs_dec_regular mpt_decode_cobs
#else
static int _decode_r
#endif
(MPT_STRUCT(decode_state) *dec, const struct iovec *source, size_t sourcelen)
{
	int ret = MPT_cobs_dec_regular(dec, source, sourcelen);
	
	/* inline zero byte */
	if (ret == MPT_ERROR(MissingData)
	    && dec->_ctx) {
		MPT_STRUCT(message) tmp;
		size_t len;
		
		len = dec->data.pos + dec->data.len;
		
		tmp.base = 0;
		tmp.used = 0;
		tmp.cont = (void *) source;
		tmp.clen = sourcelen;
		mpt_message_read(&tmp, len, 0);
		
		if (!tmp.used) {
			return MPT_ERROR(MissingBuffer);
		}
		/* add inlined end byte */
		*((uint8_t *) tmp.base) = (dec->_ctx & 0xff);
		dec->data.msg = ++dec->data.len;
		
		dec->_ctx = 0;
		++dec->curr;
		return 1;
	}
	return ret;
}
