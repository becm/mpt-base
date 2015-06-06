#include <string.h>

#include <sys/uio.h>

#include <arpa/inet.h>

#include "message.h"
#include "convert.h"

#define _MPT_COBS_MSG_COMPLETE (INTPTR_MAX ^ UINTPTR_MAX)

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
extern ssize_t mpt_decode_cobs(MPT_STRUCT(codestate) *info, const struct iovec *source, size_t sourcelen)
{
	MPT_STRUCT(message) from, to;
	int code;
	size_t flen, tlen, slen, off;
	
	if (!source) {
		if (!sourcelen) {
			info->_ctx = 0;
			info->done = 0;
			info->scratch = 0;
		}
		return 0;
	}
	
	/* decoder data sizes */
	slen = info->_ctx & ~_MPT_COBS_MSG_COMPLETE;
	off  = info->done;
	flen = info->scratch;
	
	to.base = 0;
	to.used = 0;
	to.cont = (void *) source;
	to.clen = sourcelen;
	
	/* peek at message data */
	if (!sourcelen) {
		size_t max = source->iov_len;
		
		if (!max) {
			return -2;
		}
		if (off >= max) {
			return 0;
		}
		max -= off;
		
		if (slen) {
			return slen < max ? slen : max;
		}
		to.clen = 1;
	}
	/* consume previous message */
	else if (info->_ctx & _MPT_COBS_MSG_COMPLETE) {
		size_t shift;
		
		/* find last valid part */
		if (off && (mpt_message_read(&to, off, 0) < off)) {
			return sourcelen ? -1 : 0;
		}
		shift = 0;
		while (flen > to.used) {
			shift += to.used;
			flen  -= to.used;
			mpt_message_read(&to, to.used, 0);
			if (!to.used) {
				return -1;
			}
			continue;
		}
		/* target alignment */
		for (off = 0x10; off; off /= 2) {
			const uint8_t *addr = to.base;
			size_t post = ((uintptr_t) (addr + flen)) & (off - 1);
			if (flen < post) {
				continue;
			}
			off = flen - post;
			flen = post;
			shift += off;
			break;
		}
		info->_ctx = 0;
		info->done += shift;
		info->scratch = flen;
	}
	/* skip decoded space */
	else {
		off  += slen;
		flen -= slen;
	}
	/* consume empty segments and processed data */
	if (off && (mpt_message_read(&to, off, 0) < off)) {
		return sourcelen ? -1 : 0;
	}
	/* encoded data start */
	from = to;
	if (flen && (mpt_message_read(&from, flen, 0) < flen)) {
		return sourcelen ? -1 : 0;
	}
	
	/* set start of encoded data */
	tlen = to.used;
	flen = from.used;
	
	while (1) {
		uint8_t sbuf[256], *end;
		const uint8_t *base;
		
		code = mpt_cobs_dec(from.base, &flen, (void *) to.base, &tlen);
		
		tlen = to.used - tlen;
		flen = from.used - flen;
		
		if (!code) {
			break;
		}
		info->scratch += flen;
		info->_ctx += tlen;
		
		/* tail inline */
		if (code <= -256) {
			return code;
		}
		/* consumed data */
		mpt_message_read(&from, flen, 0);
		base = from.base;
		flen = from.used;
		
		/* created data */
		mpt_message_read(&to, tlen, 0);
		end  = (void *) to.base;
		tlen = to.used;
		
		/* missing source data */
		if (code < 0) {
			MPT_STRUCT(message) tmp = from;
			size_t blen;
			
			if (!from.clen) {
				if (!sourcelen && (blen = info->_ctx)) {
					return blen;
				}
				return -2;
			}
			blen = flen = mpt_message_read(&tmp, sizeof(sbuf), sbuf);
			
			code = mpt_cobs_dec(sbuf, &flen, end, &tlen);
			blen -= flen;
			tlen = to.used - tlen;
			
			if (!code) {
				flen = blen;
				break;
			}
			info->scratch += blen;
			info->_ctx += tlen;
			
			/* inline zero byte */
			if (code <= -256) {
				return code;
			}
			/* missing source data */
			if (code < 0) {
				if (!sourcelen && (blen = info->_ctx)) {
					return blen;
				}
				return -2;
			}
			/* consume source data */
			slen = from.used;
			mpt_message_read(&from, blen, 0);
			
			/* continue from buffer */
			if (blen < slen) {
				base = sbuf + blen;
			} else {
				base = from.base;
				flen = from.used;
			}
			/* register decoded data */
			mpt_message_read(&to, tlen, 0);
		}
		/* missing target data */
		if (code > 0) {
			uint8_t tbuf[256], *buf;
			size_t tmp = flen, blen = sizeof(tbuf);
			
			code = mpt_cobs_dec(base, &flen, buf = tbuf, &blen);
			flen = tmp - flen;
			slen = sizeof(tbuf) - blen;
			
			/* save decoded data */
			off = slen;
			while (off) {
				end  = (uint8_t *) to.base;
				tlen = to.used;
				
				/* remainig data fits in current segment */
				if (tlen >= off) {
					memcpy(end, buf, off);
					to.used = tlen - off;
					to.base = end  + off;
					break;
				}
				memcpy(end, buf, tlen);
				off -= tlen;
				buf += tlen;
				
				/* advance target vector */
				do {
					const struct iovec *cont;
					if (!to.clen--) {
						return -1;
					}
					cont = to.cont++;
					to.base = cont->iov_base;
					to.used = cont->iov_len;
				} while (!tlen);
			}
			if (!code) {
				tlen = slen;
				break;
			}
			/* save decoded data */
			info->scratch += flen;
			info->_ctx += slen;
			
			tlen = to.used;
			
			/* consume source data */
			mpt_message_read(&from, flen, 0);
		}
		/* next message part */
		flen = from.used;
	}
	/* add new data */
	slen = info->_ctx + tlen;
	
	/* mark message completion */
	if (sourcelen) {
		info->_ctx = slen | _MPT_COBS_MSG_COMPLETE;
	} else {
		info->_ctx = slen;
		--flen;
	}
	/* advance start position */
	info->scratch += flen;
	
	/* return length to consume */
	return slen;
}

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
extern ssize_t mpt_decode_cobs_r(MPT_STRUCT(codestate) *info, const struct iovec *source, size_t sourcelen)
{
	ssize_t ret = mpt_decode_cobs(info, source, sourcelen);
	
	/* max. data size */
	if (ret <= -256) {
		/* save consume data size */
		ret = ++info->_ctx;
		++info->done;
		--info->scratch;
	}
	return ret;
}

/*!
 * \ingroup mptConvert
 * \brief decode COBS/ZPE data
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
extern ssize_t mpt_decode_cobs_zpe(MPT_STRUCT(codestate) *info, const struct iovec *source, size_t sourcelen)
{
	(void) info;
	(void) source;
	(void) sourcelen;
	MPT_ABORT("TODO: implement COBS/ZPE elemination");
	return -1;
}
/*!
 * \ingroup mptConvert
 * \brief decode COBS/ZPE+R data
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
 * \param mlen      consumed message length
 * \param source    encoded data
 * \param sourcelen length of source array
 * 
 * \return size of message/consumed data
 */
extern ssize_t mpt_decode_cobs_zpe_r(MPT_STRUCT(codestate) *info, const struct iovec *source, size_t sourcelen)
{
	ssize_t ret = mpt_decode_cobs_zpe(info, source, sourcelen);
	
	/* max. data size */
	if (ret <= -256) {
		/* save consume data size */
		ret = ++info->_ctx;
		++info->done;
		--info->scratch;
	}
	return ret;
}
