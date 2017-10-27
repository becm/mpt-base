/*!
 * client initialisation event.
 */

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
	MPT_INTERFACE(iterator) *it;
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
		
		/* command message */
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) >= (ssize_t) sizeof(mt)
		    && mt.cmd == MPT_ENUM(MessageCommand)
		    && (part = mpt_message_argv(&msg, mt.arg)) >= 0) {
			/* consume command part  */
			mpt_message_read(&msg, part+1, 0);
			/* create source for further arguments */
			if ((part = mpt_message_argv(&msg, mt.arg)) > 0
			    && !(src = mpt_message_iterator(&msg, mt.arg))) {
				return MPT_event_fail(ev, MPT_ERROR(BadOperation), MPT_tr("failed to create argument stream"));
			}
		}
	}
	/* initialize and bind solver */
	it = 0;
	if (src) {
		src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it);
	}
	res = cl->_vptr->init(cl, it);
	if (src) {
		src->_vptr->ref.unref((void *) src);
	}
	if (res < 0) {
		return MPT_event_fail(ev, res, MPT_tr("unable to initialize client data"));
	}
	return MPT_event_good(ev, MPT_tr("client configuration successfull"));
}

