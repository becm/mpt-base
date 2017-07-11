/*!
 * close/open message stream output,
 * dispatch command to graphic output.
 */

#include <string.h>
#include <sys/uio.h>

#include "output.h"

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief client output modification
 * 
 * Determine operations from event data.
 * 
 * \param out  output interface
 * \param ev   event data
 * 
 * \return hint to event controller (int)
 */
extern int mpt_output_control(MPT_INTERFACE(output) *out, int sep, const MPT_STRUCT(message) *mptr, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(message) msg;
	char buf[256];
	ssize_t part = -1;
	
	/* need existing output */
	if (!out || !mptr) {
		return MPT_ERROR(BadArgument);
	}
	/* get command argument */
	msg = *mptr;
	if (((part = mpt_message_argv(&msg, sep)) <= 0)) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Warning), "%s",
			        MPT_tr("missing control arguments"));
		}
		return MPT_ERROR(MissingData);
	}
	if (part >= (ssize_t) sizeof(buf)) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("unaligned command message"));
		}
		return MPT_ERROR(MissingBuffer);
	}
	mpt_message_read(&msg, part, buf);
	buf[part] = 0;
	
	/* open new connection */
	if (part >= 4 && !strncmp("open", buf, part)) {
		if ((part = mpt_message_argv(&msg, sep)) < 0) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Warning), "%s",
				        MPT_tr("missing connect argument"));
			}
			return MPT_ERROR(MissingData);
		}
		if ((size_t) part < msg.used) {
			;
		} else if (part < (ssize_t) sizeof(buf)) {
			buf[part] = 0;
			(void) mpt_message_read(&msg, part, buf);
			msg.base = buf;
		} else {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("unaligned connect argument"));
			}
			return MPT_ERROR(MissingBuffer);
		}
		if ((part = mpt_object_set_string((void *) out, 0, msg.base, 0)) < 0) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("unable to open connection"));
			}
			return MPT_ERROR(BadOperation);
		}
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Debug), "%s",
			        MPT_tr("created new connection"));
		}
		return 0;
	}
	/* command is close operation */
	else if (part >= 5 && !strncmp("close", buf, part)) {
		if ((part = mpt_object_set_string((void *) out, 0, 0, 0)) < 0) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("error on connection close"));
			}
			return MPT_ERROR(BadOperation);
		}
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Debug), "%s",
			        MPT_tr("closed connection"));
		}
		return 0;
	}
	msg = *mptr;
	/* send message data */
	if (msg.used) {
		out->_vptr->push(out, msg.used, msg.base);
	}
	while (msg.clen--) {
		if (msg.cont->iov_len) {
			out->_vptr->push(out, msg.cont->iov_len, msg.cont->iov_base);
		}
		++msg.cont;
	}
	out->_vptr->push(out, 0, 0);
	if (log) {
		mpt_log(log, __func__, MPT_LOG(Debug), "%s",
		        MPT_tr("sent command to output"));
	}
	return 0;
}

