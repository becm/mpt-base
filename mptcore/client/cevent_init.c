/*!
 * client initialisation event.
 */

#include <string.h>

#include "meta.h"
#include "message.h"
#include "event.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief call client initialisation
 * 
 * Apply client configuration from arguments in event data.
 * Initialize client data (solver binding).
 * 
 * \param cl  client descriptor
 * \param ev  event data
 * 
 * \return hint to event controller (int)
 */
extern int mpt_cevent_init(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(metatype) *src;
	int res;
	
	if (!ev) {
		return 0;
	}
	if (!cl) {
		MPT_ABORT("missing client descriptor");
	}
	/* apply command line client parameters */
	src = 0;
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		MPT_STRUCT(msgtype) mt;
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message header"));
		}
		if (mt.cmd != MPT_ENUM(MessageCommand)) {
			return MPT_event_fail(ev, MPT_ERROR(BadType), MPT_tr("bad message format"));
		}
		/* consume command part  */
		if ((part = mpt_message_argv(&msg, mt.arg)) >= 0) {
			mpt_message_read(&msg, part+1, 0);
			part = mpt_message_argv(&msg, mt.arg);
		}
		if (part >= 0 && !(src = mpt_meta_message(&msg, mt.arg))) {
			return MPT_event_fail(ev, MPT_ERROR(BadOperation), MPT_tr("failed to create argument stream"));
		}
	}
	/* initialize and bind solver */
	res = cl->_vptr->init(cl, src);
	if (src) src->_vptr->ref.unref((void *) src);
	
	if (res < 0) {
		return MPT_event_fail(ev, res, MPT_tr("unable to initialize client data"));
	}
	return MPT_event_good(ev, MPT_tr("client configuration successfull"));
}

