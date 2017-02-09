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

extern int mpt_outdata_reply(MPT_STRUCT(outdata) *out, const MPT_STRUCT(message) *src, size_t len, const void *hdr)
{
	uint8_t buf[0x10000];
	uint64_t id = 0;
	int ret;
	uint8_t ilen, slen;
	
	/* already answered */
	if (!len) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("bad reply operation"), MPT_tr("reply already sent"));
		return MPT_ERROR(BadArgument);
	}
	if (!MPT_socket_active(&out->sock)) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("bad reply operation"), MPT_tr("no connection"));
		return MPT_ERROR(BadArgument);
	}
	ilen = out->_idlen;
	slen = out->_smax;
	
	if (len > (ilen + slen)) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: %s (%u)",
		        MPT_tr("bad reply state"), MPT_tr("context data too big"), (int) len);
		return MPT_ERROR(BadArgument);
	}
	slen = len - ilen;
	
	
	if (ilen) {
		memcpy(buf, ((int8_t *) hdr) + slen, ilen);
		buf[0] &= 0x7f;
		mpt_message_buf2id(buf, ilen, &id);
		buf[0] |= 0x80;
	}
	len = 0;
	if (src) {
		MPT_STRUCT(message) msg = *src;
		len = mpt_message_read(&msg, sizeof(buf) - ilen, buf + ilen);
		if (mpt_message_length(&msg)) {
			len = 0;
			mpt_log(0, __func__, MPT_LOG(Error), "%s (%08" PRIx64 "): %s",
			        MPT_tr("unable to reply"), id, MPT_tr("message too big"));
			return MPT_ERROR(MissingBuffer);
		}
	}
	ret = sendto(out->sock._id, buf, ilen + len, 0, slen ? hdr : 0, slen);
	
	if (ret < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s (%08" PRIx64 "): %s",
		        MPT_tr("unable to reply"), id, MPT_tr("send failed"));
		return MPT_ERROR(BadArgument);
	}
	return ret;
}
