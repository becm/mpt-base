/*!
 * client initialisation event.
 */

#include <string.h>

#include "node.h"
#include "array.h"
#include "config.h"
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
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		MPT_STRUCT(msgtype) mt;
		
		if (mpt_message_read(&msg, sizeof(mt), &mt) < (ssize_t) sizeof(mt)
		    || mt.cmd != MPT_ENUM(MessageCommand)) {
			mpt_output_log(cl->out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("bad message format"));
			return -1;
		}
		if (!(src = mpt_message_metatype(mt.arg, &msg))) {
			mpt_output_log(cl->out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("failed to create argument stream"));
			return -1;
		}
	}
	mpt_output_log(cl->out, __func__, 0, 0);
	
	/* initialize and bind solver */
	res = cl->_vptr->init(cl, src);
	if (src) src->_vptr->unref(src);
	
	if (res < 0) {
		return MPT_event_fail(ev, MPT_tr("unable to initialize client data"));
	}
	return MPT_event_good(ev, MPT_tr("client configuration successfull"));
}

