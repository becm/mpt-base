/*!
 * finalize connection data
 */

#include "event.h"
#include "array.h"
#include "stream.h"

#include "message.h"

#include "output.h"

static void deregisterCommand(const _MPT_ARRAY_TYPE(command) *wait, uintptr_t id)
{
	MPT_STRUCT(command) *ans;
	
	if ((ans = mpt_command_get(wait, id))) {
		ans->cmd(ans->arg, 0);
	}
}

/*!
 * \ingroup mptOutput
 * \brief push to connection
 * 
 * Handle data push operations and state transitions.
 * 
 * \param con  connection descriptor
 * \param len  length of new data
 * \param src  data to append
 * 
 * \return state of property
 */
extern ssize_t mpt_connection_push(MPT_STRUCT(connection) *con, size_t len, const void *src)
{
	MPT_STRUCT(stream) *srm;
	ssize_t ret;
	
	/* determine active backend */
	if (MPT_socket_active(&con->out.sock)) {
		srm = 0;
	}
	else if (!(srm = (void *) con->out.buf._buf)) {
		return MPT_ERROR(BadArgument);
	}
	/* new message start */
	if (!(con->out.state & MPT_OUTFLAG(Active))
	    && con->out._idlen) {
		uint8_t buf[64];
		
		/* create normalized ID */
		if (sizeof(buf) < con->out._idlen) {
			return MPT_ERROR(BadValue);
		}
		if ((ret = mpt_message_id2buf(con->cid, buf, con->out._idlen)) < 0) {
			return ret;
		}
		/* use stream backend */
		if (srm) {
			ret = mpt_stream_push(srm, con->out._idlen, buf);
			if (ret >= 0) {
				/* force atomic id push */
				if (ret < con->out._idlen) {
					mpt_stream_push(srm, 1, 0);
					if (con->cid) {
						deregisterCommand(&con->_wait, con->cid);
					}
					return MPT_ERROR(MissingBuffer);
				} else {
					con->out.state |= MPT_OUTFLAG(Active);
				}
			}
		}
		/* use socket backend (has atomic guarantee for ID setup) */
		else if ((ret = mpt_outdata_push(&con->out, con->out._idlen, buf)) < 0) {
			if (con->cid) {
				deregisterCommand(&con->_wait, con->cid);
			}
			return ret;
		}
	}
	/* regular message payload */
	if (srm) {
		if ((ret = mpt_stream_push(srm, len, src)) < 0) {
			/* push operation failed */
			if (mpt_stream_flags(&srm->_info) & MPT_STREAMFLAG(MesgActive)) {
				mpt_stream_push(srm, 1, 0);
			}
			con->out.state &= ~MPT_OUTFLAG(Active);
		}
		/* message completed */
		else if (!len) {
			con->out.state &= ~MPT_OUTFLAG(Active);
			mpt_stream_flush(srm);
			con->cid = 0;
		} else {
			con->out.state |= MPT_OUTFLAG(Active);
		}
	}
	else if ((ret = mpt_outdata_push(&con->out, len, src)) < 0) {
		if (con->out.state & MPT_OUTFLAG(Active)) {
			mpt_outdata_push(&con->out, 1, 0);
		}
	}
	if (ret < 0 && con->cid) {
		/* clear pending reply */
		deregisterCommand(&con->_wait, con->cid);
	}
	return ret;
}
