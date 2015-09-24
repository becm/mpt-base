#include <string.h>

#include <sys/uio.h>

#include <arpa/inet.h>

#include "message.h"
#include "convert.h"

#define _MPT_COBS_MSG_COMPLETE (INTPTR_MAX ^ UINTPTR_MAX)
#define _MPT_COBS_MLEN_FACT    0x10000

#ifndef MPT_cobs_msg_state
# define MPT_cobs_msg_state(len, code, pos) ((len * _MPT_COBS_MLEN_FACT) + (pos) * 0x100 + (code))
#endif

#ifndef MPT_cobs_data_len
# define MPT_cobs_data_len(c)   ((c)-1)
#endif

#ifndef MPT_cobs_zero_len
# define MPT_cobs_zero_len(c,n) ((((c) < MPT_COBS_MAXLEN) && n) ? 1 : 0)
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
 * New message data begins at \info.done ,
 * encoded data continues \info.scratch later
 * 
 * Pass \sourcelen = 0 to get encoded message length
 * on single data element.
 * No data change is performed in this case.
 * 
 * \param info      start position of encoded data
 * \param source    encoded data
 * \param sourcelen length of source array
 * 
 * \return size of message/consumed data
 */
extern ssize_t mpt_decode_cobs
#define MPT_COBS_MAXLEN 255
#else
static ssize_t _decode
#endif
(MPT_STRUCT(codestate) *info, const struct iovec *source, size_t sourcelen)
{
	MPT_STRUCT(message) tmp;
	const struct iovec *dvec;
	size_t dveclen, dlen, clen;
	const uint8_t *restrict src;
	uint8_t *restrict dst, code, pos;
	size_t proc, mlen, done, rem;
	
	/* used message buffer */
	mlen = (info->_ctx & ~_MPT_COBS_MSG_COMPLETE) / _MPT_COBS_MLEN_FACT;
	
	if (!source) {
		if (!sourcelen) {
			info->_ctx = 0;
		} else {
			return MPT_cobs_max_dec(sourcelen) + mlen;
		}
		return 0;
	}
	
	/* get minimum scratch area */
	if (!sourcelen) {
		return mlen;
	}
	/* decoder data sizes */
	code = info->_ctx & 0xff;
	done = info->done;
	proc = info->scratch;
	
	tmp.base = 0;
	tmp.used = 0;
	tmp.cont = (void *) source;
	tmp.clen = sourcelen;
	
	/* check info for data vector */
	dlen = mlen + done;
	if ((mlen > proc)
	    || (mpt_message_read(&tmp, dlen, 0) < dlen)) {
		return MPT_ERROR(BadArgument);
	}
	/* space available for target data */
	proc -= mlen;
	
	/* consume previous message */
	if (info->_ctx & _MPT_COBS_MSG_COMPLETE) {
		const uint8_t *addr;
		size_t curr, align = 0x10;
		
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
		info->_ctx = 0;
		info->done = done;
		info->scratch = proc;
		mlen = 0;
	}
	if ((rem = mpt_message_length(&tmp)) < proc) {
		return MPT_ERROR(BadArgument);
	}
	rem -= proc;
	
	/* decoded data start */
	dst  = (uint8_t *) tmp.base;
	dlen = tmp.used;
	dvec = tmp.cont;
	dveclen = tmp.clen;
	
	/* encoded data start */
	mpt_message_read(&tmp, proc, 0);
	src  = tmp.base;
	clen = tmp.used;
	
	/* finished with complete block/message */
	if (!code) {
		if (!mpt_message_read(&tmp, 1, &code)) {
			return MPT_ERROR(MissingData);
		}
		src  = tmp.base;
		clen = tmp.used;
		++proc;
		
		/* double/leading zero */
		if (!code) {
			info->_ctx |= _MPT_COBS_MSG_COMPLETE;
			return MPT_ERROR(BadValue);
		}
	}
	/* position in last save operation */
	pos = (info->_ctx & 0xff00) / 0x100;
	
	while (1) {
		uint8_t curr, next;
		
		/* message finished */
		if (!code) {
			/* illegal message sizes */
			if (mlen > UINTPTR_MAX/_MPT_COBS_MLEN_FACT) {
				return MPT_ERROR(BadOperation);
			}
			info->_ctx = (mlen * _MPT_COBS_MLEN_FACT) | _MPT_COBS_MSG_COMPLETE;
			info->scratch = proc + mlen;
			return mlen;
		}
		/* length of non-zero part */
		curr = MPT_cobs_data_len(code);
		
		for (; pos < curr; ++pos) {
			uint8_t val;
			while (!clen--) {
				if (!tmp.clen--) {
					info->_ctx = MPT_cobs_msg_state(mlen, code, pos);
					info->scratch = proc + mlen;
					return MPT_ERROR(MissingData);
				}
				src  = tmp.cont->iov_base;
				clen = tmp.cont->iov_len;
				++tmp.cont;
			}
			/* regular data */
			if ((val = *src++)) {
			/* no remaining target space */
				if (!proc) {
					info->_ctx = MPT_cobs_msg_state(mlen, code, pos);
					info->scratch = proc + mlen;
					return MPT_ERROR(MissingBuffer);
				}
				/* no remaining data on current part */
				while (!dlen--) {
					dst  = (uint8_t *) dvec->iov_base;
					dlen = dvec->iov_len;
					++dvec;
					--dveclen;
				}
				*dst++ = val;
				++mlen;
				continue;
			}
			/* tail byte inline */
			info->_ctx = MPT_cobs_msg_state(mlen, code, pos);
			info->scratch = proc + mlen + 1;
			return MPT_ERROR(BadEncoding);
		}
		/* read next element */
		while (!clen--) {
			if (!tmp.clen--) {
				info->_ctx = MPT_cobs_msg_state(mlen, code, pos);
				info->scratch = proc + mlen;
				return MPT_ERROR(MissingData);
			}
			src  = tmp.cont->iov_base;
			clen = tmp.cont->iov_len;
			++tmp.cont;
		}
		++proc;
		next = *src++;
		
		/* needed numbers of zeros */
		curr += MPT_cobs_zero_len(code, next);
		
		for (; pos < curr; ++pos) {
			/* no remaining target space */
			if (!proc) {
				info->_ctx = MPT_cobs_msg_state(mlen, code, pos);
				info->scratch = proc + mlen;
				return MPT_ERROR(MissingBuffer);
			}
			/* no remaining data on current part */
			while (!dlen--) {
				dst  = (uint8_t *) dvec->iov_base;
				dlen = dvec->iov_len;
				++dvec;
				--dveclen;
			}
			*dst++ = 0;
			++mlen;
			--proc;
		}
		/* start next part */
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
 * New message data begins at \info.done ,
 * encoded data continues \info.scratch later
 * 
 * Pass \sourcelen = 0 to get encoded message length
 * on single data element.
 * No data change is performed in this case.
 * 
 * \param info      start position of encoded data
 * \param source    encoded data
 * \param sourcelen length of source array
 * 
 * \return size of message/consumed data
 */
extern ssize_t mpt_decode_cobs_r
# define MPT_cobs_dec_regular mpt_decode_cobs
#else
static ssize_t _decode_r
#endif
(MPT_STRUCT(codestate) *info, const struct iovec *source, size_t sourcelen)
{
	ssize_t ret = MPT_cobs_dec_regular(info, source, sourcelen);
	uint8_t val;
	
	/* max. data size */
	if (ret == MPT_ERROR(BadEncoding)
	    && ((val = info->_ctx & 0xff) > 1)) {
		MPT_STRUCT(message) tmp;
		
		ret = info->_ctx / _MPT_COBS_MLEN_FACT;
		
		tmp.base = 0;
		tmp.used = 0;
		tmp.cont = (void *) source;
		tmp.clen = sourcelen;
		mpt_message_read(&tmp, ret + info->done, 0);
		
		if (!tmp.used) {
			return MPT_ERROR(MissingBuffer);
		}
		/* save consume data size */
		*((uint8_t *) tmp.base) = val;
		++ret;
		
		info->_ctx = (ret * _MPT_COBS_MLEN_FACT) | _MPT_COBS_MSG_COMPLETE;
	}
	return ret;
}
