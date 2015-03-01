/*!
 * close/open message stream output,
 * dispatch command to graphic output.
 */

#include <errno.h>
#include <string.h>
#include <sys/uio.h>

#include "message.h"
#include "event.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief client output modification
 * 
 * Determine operations from event data.
 * 
 * \param solv  client descriptor
 * \param ev    event data
 * 
 * \return hint to event controller (int)
 */
extern int mpt_cevent_graphic(MPT_INTERFACE(client) *solv, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(output) *out;
	MPT_STRUCT(msgtype) mt;
	MPT_STRUCT(message) msg, tmp;
	MPT_STRUCT(property) pr;
	char buf[256];
	ssize_t part = -1;
	
	if (!ev) return 0;
	
	/* graphic command needs arguments */
	if (!ev->msg) {
		return MPT_event_fail(ev, MPT_tr("missing data for graphic command"));
	}
	/* get command type */
	msg = *ev->msg;
	if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)
	    || mt.cmd != MPT_ENUM(MessageCommand)) {
		mpt_output_log(solv->out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("bad message format"));
		return -1;
	}
	/* need existing output */
	if (!solv || !(out = solv->out)) {
		errno = EFAULT;
		return MPT_event_fail(ev, MPT_tr("unable to configure output"));
	}
	/* first argument (consume command) */
	if (((part = mpt_message_argv(&msg, mt.arg)) <= 0)
	    || (mpt_message_read(&msg, part, 0) < (size_t) part)
	    || ((part = mpt_message_argv(&msg, mt.arg)) <= 0)) {
		return MPT_event_fail(ev, MPT_tr("missing graphic arguments"));
	}
	if (part >= (ssize_t) sizeof(buf)) {
		return MPT_event_fail(ev, MPT_tr("unaligned command message"));
	}
	tmp = msg;
	mpt_message_read(&msg, part, buf);
	buf[part] = 0;
	
	/* open new connection */
	if (part >= 4 && !strncmp("open", buf, part)) {
		if ((part = mpt_message_argv(&msg, mt.arg)) < 0) {
			return MPT_event_fail(ev, MPT_tr("missing connect argument"));
		}
		pr.name = "";
		pr.desc = 0;
		pr.fmt  = 0;
		if ((size_t) part < msg.used) {
			pr.data = msg.base;
		} else if (part < (ssize_t) sizeof(buf)) {
			buf[part] = 0;
			part = mpt_message_read(&msg, part, buf);
			pr.data = buf;
		} else {
			return MPT_event_fail(ev, MPT_tr("unaligned connect argument"));
		}
		
		if (mpt_meta_pset((void *) out, &pr, 0) < 0) {
			return MPT_event_fail(ev, MPT_tr("unable to open connection"));
		}
		return MPT_event_good(ev, MPT_tr("created new graphic connection"));
	}
	/* command is close operation */
	else if (part >= 5 && !strncmp("close", buf, part)) {
		pr.name = "";
		pr.desc = 0;
		pr.fmt  = 0;
		pr.data = 0;
		
		if (mpt_meta_pset((void *) out, &pr, 0) < 0) {
			return MPT_event_fail(ev, MPT_tr("error on graphic close"));
		}
		return MPT_event_good(ev, MPT_tr("closed graphic connection"));
	}
	msg = tmp;
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
	
	return MPT_event_good(ev, MPT_tr("sent command to graphic"));
}

