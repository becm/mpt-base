/*!
 * text to command
 */

#include <string.h>

#include <sys/uio.h>

#include "message.h"
#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief command decoder
 * 
 * Convert zero-delimited text to command message
 * by prepending message header to segments.
 * 
 * \param off  start of source data
 * \param src  encoded data
 * \param len  number of data elements
 * 
 * \return consumed data size
 */
extern int mpt_decode_command(MPT_STRUCT(decode_state) *dec, const struct iovec *source, size_t sourcelen)
{
	MPT_STRUCT(message) from;
	size_t off, len, pos;
	uint8_t *end;
	
	if (!source) {
		static const MPT_STRUCT(decode_state) _def = MPT_DECODE_INIT;
		*dec = _def;
		return 0;
	}
	pos = dec->curr;
	len = dec->data.len;
	off = dec->data.pos;
	
	if (dec->data.msg >= 0) {
		if (!sourcelen) {
			return MPT_ERROR(BadOperation);
		}
		len -= dec->data.msg;
	}
	from.used = 0;
	from.base = 0;
	from.cont = (void *) source;
	from.clen = sourcelen ? sourcelen : 1;
	
	/* start new message */
	if (!len) {
		static MPT_STRUCT(msgtype) mt = { MPT_MESGTYPE(Command), ' ' };
		
		if (!sourcelen) {
			return MPT_ERROR(BadOperation);
		}
		/* need place for header */
		if (pos < sizeof(mt)) {
			return MPT_ERROR(MissingBuffer);
		}
		/* start with header */
		off = pos - sizeof(mt);
		if (mpt_message_read(&from, off, 0) < off) {
			return MPT_ERROR(MissingData);
		}
		/* save command header */
		end = (uint8_t *) from.base;
		while (len < sizeof(mt)) {
			const struct iovec *cont;
			
			if (from.used--) {
				*(end++) = ((uint8_t *) &mt)[len++];
				continue;
			}
			if (!from.clen--) {
				return MPT_ERROR(MissingBuffer);
			}
			cont = from.cont++;
			from.base = end = cont->iov_base;
			from.used = cont->iov_len;
		}
		dec->data.pos = off;
		dec->data.len = len;
		dec->data.msg = -1;
	}
	/* check persistent data assumption */
	else if (pos != (off + len)) {
		return MPT_ERROR(BadArgument);
	}
	/* skip checked data */
	else if (mpt_message_read(&from, pos, 0) < pos) {
		return MPT_ERROR(MissingData);
	}
	/* find command end */
	while (1) {
		if (from.used && (end = memchr(from.base, 0, from.used))) {
			len += (end - (uint8_t *) from.base);
			
			if (!sourcelen || dec->data.msg >= 0) {
				dec->curr = off + len;
				dec->data.len = len;
				return 1;
			}
			dec->data.len = len;
			dec->data.msg = len;
			
			dec->curr = off + len + 1;
			return 1;
		}
		len += from.used;
		if (!from.clen--) {
			dec->data.len = len;
			dec->curr = off + len;
			return 0;
		}
		from.base = from.cont->iov_base;
		from.used = from.cont->iov_len;
		++from.cont;
	}
}
