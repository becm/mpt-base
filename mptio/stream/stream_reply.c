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

#include "message.h"
#include "stream.h"

extern int mpt_stream_reply(MPT_STRUCT(stream) *srm, const MPT_STRUCT(message) *msg, size_t len, const void *val)
{
	uint64_t id = 0;
	int ret;
	
	if (mpt_stream_flags(&srm->_info) & MPT_STREAMFLAG(MesgActive)) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s (%08"PRIx64"): %s",
		        MPT_tr("unable to reply"), id, MPT_tr("message creation in progress"));
		return MPT_ERROR(BadArgument);
	}
	if (len) {
		uint8_t first;
		mpt_message_buf2id(val, len, &id);
		first = *((uint8_t *) val) | 0x80;
		
		if ((ret = mpt_stream_push(srm, 1, &first)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s (%08"PRIx64"): %s",
			        MPT_tr("bad reply operation"), id, MPT_tr("unable to start reply"));
			return ret;
		}
		if ((ret = mpt_stream_push(srm, len - 1, ((uint8_t *) val) + 1)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s (%08"PRIx64"): %s",
			        MPT_tr("bad reply operation"), id, MPT_tr("unable to start reply"));
			mpt_stream_push(srm, 1, 0);
			return ret;
		}
	}
	if (msg && mpt_stream_append(srm, msg) < 0) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s (%08"PRIx64"): %s",
		        MPT_tr("bad reply operation"), id, MPT_tr("unable to append message"));
		if (len && mpt_stream_push(srm, 1, 0) >= 0) {
			return MPT_ERROR(BadOperation);
		}
		len += mpt_message_length(msg);
	}
	if ((ret = mpt_stream_push(srm, 0, 0)) < 0) {
		mpt_log(0, __func__, MPT_LOG(Critical), "%s (%08"PRIx64"): %s",
		        MPT_tr("bad reply operation"), id, MPT_tr("unable to terminate reply"));
		if (len && mpt_stream_push(srm, 1, 0) < 0) {
			return ret;
		}
	}
	return 0;
}
