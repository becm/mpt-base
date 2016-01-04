/*!
 * finalize connection data
 */

#include <stdio.h>

#include <arpa/inet.h>

#include "array.h"
#include "queue.h"
#include "event.h"

#include "message.h"

#include "output.h"

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
			if (!(ret = mpt_outdata_print(&con->out, con->hist.file, len, src))) {
				ret = len;
			}
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
			con->out.state &= MPT_ENUM(OutputPrintColor);
			con->hist.info.size = 0;
			con->cid = 0;
		}
		return ret;
	}
	if (!src) {
		return -2;
	}
	if (len > 1 && !(con->out.state & MPT_ENUM(OutputRemote))) {
		const MPT_STRUCT(msgtype) *mt = src;
		if ((ret = mpt_outdata_print(&con->out, con->hist.file, len, src)) >= 0) {
			return ret ? ret : (ssize_t) len;
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
	/* TODO: semantics to skip ID */
	if (!(con->out.state & 0x40)) {
		struct iovec vec;
		uint16_t mid = htons(con->cid);
		
		vec.iov_base = &mid;
		vec.iov_len  = sizeof(mid);
		
		if (MPT_socket_active(&con->out.sock)
		    && (ret = mpt_array_push(&con->out._buf, &con->out._enc.info, con->out._enc.fcn, &vec)) < (ssize_t) sizeof(mid)) {
			return -3;
		}
	}
	con->out.state &= ~MPT_ENUM(OutputRemote);
	if ((ret = mpt_outdata_push(&con->out, len, src)) < 0) {
		MPT_STRUCT(command) *cmd;
		if (con->cid && (cmd = mpt_command_get(&con->_wait, con->cid))) {
			cmd->cmd(cmd->arg, 0);
			cmd->cmd = 0;
		}
	}
	return ret;
}
