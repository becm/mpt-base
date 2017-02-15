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

extern int mpt_connection_reply(MPT_STRUCT(connection) *con, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(reply_context) *rc;
	MPT_STRUCT(reply_data) *rd;
	uint64_t id = 0;
	int ret;
	
	if (!(rc = con->_rctx)) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s %s",
		        MPT_tr("unable to reply"), id, MPT_tr("no reply context available"));
		return 0;
	}
	rd = (void *) (rc + 1);
	
	/* already answered */
	if (!rd->len) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("bad reply operation"), MPT_tr("reply already sent"));
		return MPT_ERROR(BadArgument);
	}
	if (!MPT_socket_active(&con->out.sock)) {
		MPT_STRUCT(stream) *srm;
		rd->val[0] &= 0x7f;
		mpt_message_buf2id(rd->val, rd->len, &id);
		
		if (!(srm = (void *) con->out.buf._buf)) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s (%08" PRIx64 "): %s",
			        MPT_tr("unable to reply"), id, MPT_tr("no target descriptor"));
			return MPT_ERROR(BadArgument);
		}
		ret = mpt_stream_reply(srm, msg, rd->len, rd->val);
		
		if (mpt_stream_flags(&srm->_info) & MPT_STREAMFLAG(MesgActive)) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s (%08" PRIx64 "): %s",
			        MPT_tr("unable to reply"), id, MPT_tr("message creation in progress"));
			return MPT_ERROR(BadArgument);
		}
		if ((ret = mpt_stream_push(srm, rd->len, rd->val)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s (%08" PRIx64 "): %s",
			        MPT_tr("bad reply operation"), id, MPT_tr("unable to start reply"));
			return ret;
		}
		rd->len = 0;
		if (msg && mpt_stream_append(srm, msg) < 0) {
			mpt_log(0, __func__, MPT_LOG(Warning), "%s (%08" PRIx64 "): %s",
			        MPT_tr("bad reply operation"), id, MPT_tr("unable to append message"));
		}
		if ((ret = mpt_stream_push(srm, 0, 0)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s (%08" PRIx64 "): %s",
			        MPT_tr("bad reply operation"), id, MPT_tr("unable to terminate reply"));
			if (mpt_stream_push(srm, 1, 0) < 0) {
				return ret;
			}
		}
	}
	else {
		ret = mpt_outdata_reply(&con->out, msg, rd->len, rd->val);
	}
	if (ret >= 0) {
		rd->len = 0;
	}
	return ret;
}
