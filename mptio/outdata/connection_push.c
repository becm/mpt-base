/*!
 * finalize connection data
 */

#include <stdio.h>

#include <sys/uio.h>

#include <arpa/inet.h>

#include "event.h"
#include "message.h"

#include "array.h"

#include "output.h"

#define AnswerFlags(a) ((a & 0xf0) >> 4)
#define OutputFlags(a) (a & 0xf)

static int answerType(int code)
{
	if (!code) {
		return MPT_LOG(Debug);
	}
	else if (code < 0) {
		return MPT_LOG(Error);
	}
	return MPT_LOG(Info);
}

/*!
 * \ingroup mptOutput
 * \brief push to connection
 * 
 * Get property for connection or included outdata.
 * 
 * \param con  connection descriptor
 * \param len  length of new data
 * \param src  data to append
 * 
 * \return state of property
 */
extern ssize_t mpt_connection_push(MPT_STRUCT(connection) *con, size_t len, const void *src)
{
	ssize_t ret;
	
	/* message in progress */
	if (con->out.state & MPT_ENUM(OutputActive)) {
		/* local print condition */
		if (con->out.state & 0x7) {
			return mpt_outdata_print(&con->out.state, con->hist.file, len, src);
		}
		/* history output triggered */
		else if (con->hist.info.lfmt) {
			ret = mpt_history_print(con->hist.file, &con->hist.info, len, src);
		}
		else {
			ret = mpt_outdata_push(&con->out, len, src);
		}
		if (ret < 0) {
			return ret;
		}
		if (!len) {
			con->cid = 0;
			con->out.state &= MPT_ENUM(OutputPrintColor);
			con->hist.info.lfmt = 0;
		}
		return ret;
	}
	if (!src) {
		return MPT_ERROR(BadOperation);
	}
	/* message in progress */
	if (con->out.state & MPT_ENUM(OutputReceived)) {
		return MPT_ERROR(MessageInProgress);
	}
	/* local filter for message */
	if (!(con->out.state & MPT_ENUM(OutputRemote))) {
		const MPT_STRUCT(msgtype) *mt = src;
		int type, flags = -1;
		
		/* convert history to printable output */
		if (mt->cmd == MPT_ENUM(MessageValFmt)) {
			/* reset history state */
			mpt_history_reset(&con->hist.info);
			con->hist.info.lfmt = -1;
			
			/* assume block termination */
			if (len < 2) {
				con->hist.info.pos.elem = 1;
				con->out.state |= MPT_ENUM(OutputActive);
				return 1;
			}
			con->hist.info.pos.fmt = mt->arg;
			
			if (!con->hist.file) {
				con->out.state |= MPT_ENUM(OutputActive);
				return 0;
			}
			ret = 0;
			if ((len -= 2)
			    && (ret = mpt_history_print(con->hist.file, &con->hist.info, len, mt+1)) < 0) {
				return ret;
			}
			con->out.state |= MPT_ENUM(OutputActive);
			
			return ret + 2;
		}
		if (len < 2) {
			return MPT_ERROR(MissingData);
		}
		/* setup answer output */
		if (mt->cmd == MPT_ENUM(MessageOutput)) {
			type  = mt->arg;
			flags = OutputFlags(con->level);
		}
		else if (mt->cmd == MPT_ENUM(MessageAnswer)) {
			type  = answerType(mt->arg);
			flags = AnswerFlags(con->level);
		}
		if (flags >= 0) {
			flags = mpt_outdata_type(type, flags);
			
			if (!flags) {
				flags = MPT_ENUM(OutputPrintRestore);
			}
			con->out.state = (con->out.state & ~0x7) | (flags & 0x7);
			
			return mpt_outdata_print(&con->out.state, con->hist.file, len, src);
		}
	}
	/* prepend message ID */
	if (con->out._idlen) {
		uint8_t buf[sizeof(uintptr_t)];
		
		if ((ret = mpt_message_id2buf(con->cid, buf, con->out._idlen)) < 0) {
			return ret;
		}
		if ((ret = mpt_outdata_push(&con->out, con->out._idlen, buf)) < 0) {
			return ret;
		}
	}
	if ((ret = mpt_outdata_push(&con->out, len, src)) < 0) {
		mpt_outdata_push(&con->out, 1, 0);
	}
	return ret;
}
