/*!
 * dispatch event or reply from connection input
 */

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "output.h"

#include "message.h"
#include "stream.h"

extern int mpt_outdata_reply(MPT_STRUCT(outdata) *out, size_t len, const void *hdr, const MPT_STRUCT(message) *src)
{
	uint8_t tmp[0x100]; /* 256b reply limit */
	uint8_t *ptr = tmp;
	int ret;
	uint8_t ilen, slen;
	uint16_t max;
	
	/* already answered */
	if (!MPT_socket_active(&out->sock)) {
		return MPT_ERROR(BadArgument);
	}
	ilen = out->_idlen;
	slen = out->_smax;
	max = ilen + slen;
	
	if (len > max || len < ilen) {
		return MPT_ERROR(BadValue);
	}
	if (ilen) {
		memcpy(tmp, hdr, ilen);
		hdr = ((uint8_t *) hdr) + ilen;
	}
	if (!slen || !(slen = len - ilen)) {
		hdr = 0;
	}
	len = 0;
	if (src) {
		MPT_STRUCT(message) msg = *src;
		size_t left;
		len = mpt_message_read(&msg, sizeof(tmp) - ilen, tmp + ilen);
		/* temporary reply limit exceeded */
		if ((left = mpt_message_length(&msg))) {
			len += ilen + left;
			/* use temporary data in unused buffer segment */
			if (!(ptr = mpt_array_append(&out->buf, len, 0))) {
				return MPT_ERROR(MissingBuffer);
			}
			out->buf._buf->_used -= len;
			if (ilen) {
				memcpy(ptr, hdr, ilen);
			}
			len = mpt_message_read(&msg, len - ilen, ptr + ilen);
		}
	}
	ret = sendto(out->sock._id, ptr, ilen + len, 0, hdr, slen);
	
	if (ret < 0) {
		return MPT_ERROR(BadOperation);
	}
	return ret;
}
