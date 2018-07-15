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
extern ssize_t mpt_decode_command(MPT_STRUCT(decode_state) *info, const struct iovec *source, size_t sourcelen)
{
	static MPT_STRUCT(msgtype) mt = { MPT_MESGTYPE(Command), ' ' };
	MPT_STRUCT(message) from;
	size_t off, len, pos;
	uint8_t *end;
	
	if (!source) {
		static const MPT_STRUCT(decode_state) _def = MPT_DECODE_INIT;
		*info = _def;
		return 0;
	}
	len = info->_ctx;
	off = info->content.pos;
	pos = info->work.pos;
	
	/* peek at data */
	if (!sourcelen) {
		size_t max = source->iov_len;
		
		if (info->content.len >= 0) {
			return MPT_ERROR(BadOperation);
		}
		if (off < sizeof(mt)) {
			return MPT_ERROR(MissingBuffer);
		}
		if (max < off) {
			return MPT_ERROR(MissingData);
		}
		max -= off;
		if (len < max) {
			return max;
		}
		return len;
	}
	
	from.used = 0;
	from.base = 0;
	from.cont = (void *) source;
	from.clen = sourcelen ? sourcelen : 1;
	
	/* start new message */
	if (!len) {
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
				return MPT_ERROR(BadOperation);
			}
			cont = from.cont++;
			from.base = end = cont->iov_base;
			from.used = cont->iov_len;
		}
		info->_ctx = len;
		info->content.pos = off;
	}
	/* skip checked data */
	else if (mpt_message_read(&from, off, 0) < off) {
		return MPT_ERROR(MissingData);
	}
	/* find command end */
	while (1) {
		if (from.used && (end = memchr(from.base, 0, from.used))) {
			len += (end - (uint8_t *) from.base);
			break;
		}
		len += from.used;
		if (!from.clen--) {
			info->_ctx = len;
			info->work.pos = off + len;
			return MPT_ERROR(MissingData);
		}
		from.base = from.cont->iov_base;
		from.used = from.cont->iov_len;
		++from.cont;
	}
	
	info->_ctx = 0;
	info->work.pos = off + len + 1;
	
	return len;
}
