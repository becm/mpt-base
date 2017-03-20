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
	static MPT_STRUCT(msgtype) mt = { MPT_ENUM(MessageCommand), ' ' };
	MPT_STRUCT(message) from;
	size_t off, len, pos;
	uint8_t *end;
	
	if (!source) {
		info->_ctx = 0;
		info->done = 0;
		info->scratch = 0;
		return 0;
	}
	len = info->_ctx;
	off = info->done;
	pos = info->scratch;
	
	/* peek at data */
	if (!sourcelen) {
		size_t max = source->iov_len;
		
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
		if ((off + pos) < sizeof(mt)) {
			return MPT_ERROR(MissingBuffer);
		}
		/* offset is start of message header */
		if (pos < sizeof(mt)) {
			off -= sizeof(mt) - pos;
			pos = sizeof(mt);
		} else {
			off += pos - sizeof(mt);
		}
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
		info->done = off;
		info->scratch = len;
	}
	/* register additional data */
	else {
		/* scratch space is message */
		if (len != pos) {
			return MPT_ERROR(BadArgument);
		}
		off += pos;
		if (mpt_message_read(&from, off, 0) < off) {
			return MPT_ERROR(MissingData);
		}
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
			info->scratch = len;
			return MPT_ERROR(MissingData);
		}
		from.base = from.cont->iov_base;
		from.used = from.cont->iov_len;
		++from.cont;
	}
	
	info->_ctx = 0;
	info->scratch = len + 1;
	
	return len;
}
