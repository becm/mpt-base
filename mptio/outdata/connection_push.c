/*!
 * finalize connection data
 */

#include <stdio.h>

#include <sys/uio.h>

#include <arpa/inet.h>

#include "array.h"
#include "queue.h"
#include "event.h"

#include "message.h"

#include "output.h"

#define AnswerFlags(a) ((a & 0xf0) >> 4)
#define OutputFlags(a) (a & 0xf)

static int answerType(int code)
{
	if (!code) {
		return MPT_ENUM(LogDebug);
	}
	else if (code < 0) {
		return MPT_ENUM(LogError);
	}
	return MPT_ENUM(LogInfo);
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
		else if (con->hist.info.size) {
			if (con->hist.info.type) {
				ret = mpt_history_print(con->hist.file, &con->hist.info, len, src);
			} else {
				MPT_STRUCT(histinfo) info = MPT_HISTINFO_INIT;
				info.type = 't';
				info.pos = con->hist.info.pos;
				info.size = sizeof(uint64_t);
				ret = mpt_history_print(con->hist.file, &info, len, src);
				con->hist.info.pos = info.pos;
			}
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
			con->hist.info.size = 0;
		}
		return ret;
	}
	if (!src) {
		return -2;
	}
	if (len > 1 && !(con->out.state & MPT_ENUM(OutputRemote))) {
		const MPT_STRUCT(msgtype) *mt = src;
		int type, flags = -1;
		
		if (!mt) {
			return MPT_ERROR(BadOperation);
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
		/* convert history to printable output */
		if (mt->cmd == MPT_ENUM(MessageValFmt)) {
			size_t parts = mt->arg;
			ret = 2 + parts * 2;
			if (len < (size_t) ret) {
				return -2;
			}
			mpt_history_set(&con->hist.info, 0);
			
			if (!parts) {
				con->hist.info.size = sizeof(uint64_t);
			}
			else while (parts--) {
				if (mpt_history_set(&con->hist.info, (void *) (++mt)) < 0) {
					con->hist.info.line = 0;
					con->hist.info.type = 0;
				}
			}
			con->out.state |= MPT_ENUM(OutputActive);
			
			/* consume data for bad setup */
			if (!con->hist.info.size
			    || ((parts = (len - ret))
			        && (ret = mpt_history_print(con->hist.file, &con->hist.info, parts, src)) < 0)) {
				con->out.state |= MPT_ENUM(OutputPrintRestore);
				return 0;
			}
			return len;
		}
	}
	/* prepend message ID */
	if (con->out._idlen) {
		uint8_t buf[sizeof(uintptr_t)];
		
		if ((ret = mpt_message_id2buf(buf, con->out._idlen, con->cid)) < 0) {
			return ret;
		}
		if ((ret = mpt_outdata_push(&con->out, con->out._idlen, buf)) < 0) {
			return ret;
		}
	}
	con->out.state &= ~MPT_ENUM(OutputRemote);
	if ((ret = mpt_outdata_push(&con->out, len, src)) < 0) {
		mpt_outdata_push(&con->out, 1, 0);
	}
	return ret;
}
