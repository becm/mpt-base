/*!
 * dispatch event or reply from connection input
 */

#include "message.h"
#include "stream.h"

extern int mpt_stream_reply(MPT_STRUCT(stream) *srm, size_t len, const void *val, const MPT_STRUCT(message) *msg)
{
	ssize_t ret;
	
	if (mpt_stream_flags(&srm->_info) & MPT_STREAMFLAG(MesgActive)) {
		return MPT_ERROR(BadArgument);
	}
	if (len && (ret = mpt_stream_push(srm, len, val)) < 0) {
		return ret;
	}
	if (msg) {
		if ((ret = mpt_stream_append(srm, msg)) < 0) {
			if (len) {
				mpt_stream_push(srm, 1, 0);
			}
			return ret;
		}
		len += ret;
		if (ret < (ssize_t) mpt_message_length(msg)) {
			if (len) {
				mpt_stream_push(srm, 1, 0);
			}
			return MPT_ERROR(MissingBuffer);
		}
	}
	if ((ret = mpt_stream_push(srm, 0, 0)) < 0) {
		if (len) {
			mpt_stream_push(srm, 1, 0);
		}
		return ret;
	}
	return 0;
}
