#include <errno.h>
#include <string.h>

#include <sys/uio.h>

#include "message.h"
#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief command format
 * 
 * Convert data to command by adding header at
 * beginning.
 * 
 * \param off  start of source data
 * \param src  encoded data
 * \param len  number of data elements
 * 
 * \return consumed data size
 */
extern ssize_t mpt_decode_command(MPT_STRUCT(codestate) *info, const struct iovec *source, size_t sourcelen)
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
		pos = source->iov_len;
		
		if (!len) {
			return -2;
		}
		if (pos < off || len != off) {
			return 0;
		}
		pos -= off;
		
		if (pos < len) {
			len = pos;
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
			return -3;
		}
		if (pos < sizeof(mt)) {
			off -= sizeof(mt) - pos;
			pos = sizeof(mt);
		}
		
		off += pos;
		
		if (off && (mpt_message_read(&from, off, 0) < off)) {
			return -2;
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
				return -2;
			}
			cont = from.cont++;
			from.base = end = cont->iov_base;
			from.used = cont->iov_len;
		}
		info->_ctx = len;
		info->done = off;
		info->scratch = pos;
	}
	/* register additional data */
	else {
		off += pos;
		if (off && mpt_message_read(&from, off, 0) < off) {
			return -3;
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
			return -2;
		}
		from.base = from.cont->iov_base;
		from.used = from.cont->iov_len;
		++from.cont;
	}
	++len;
	
	info->_ctx = 0;
	info->done = off + len;
	info->scratch = 0;
	
	return len;
}
