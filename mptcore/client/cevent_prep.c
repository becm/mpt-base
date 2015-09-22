/*!
 * configure and prepare bound solver.
 */

#include <string.h>
#include <errno.h>

#include "array.h"
#include "message.h"
#include "event.h"

#include "client.h"


static int sliceConv(MPT_INTERFACE(source) *src, int type, void *data)
{
	MPT_INTERFACE(slice) *s = (void *) (src+1);
	char *base, *end;
	size_t len;
	
	if (!s->_len) {
		return 0;
	}
	if (!(base = mpt_array_slice(&s->_a, s->_off, s->_len))) {
		MPT_ABORT("invalid slice parameters");
	}
	if (!(end = memchr(base, 0, s->_len))) {
		return -2;
	}
	len = end - base;
	
	if (type == MPT_ENUM(TypeProperty)) {
		if (!(end = memchr(base, '=', len++))) {
			return -3;
		}
		*end = 0;
		
		if (data) {
			MPT_STRUCT(property) *pr = data;
			
			pr->name = base;
			pr->desc = 0;
			pr->val.fmt = 0;
			pr->val.ptr = end + 1;
			s->_off += len;
			s->_len -= len;
		}
	}
	else if (type != 's') {
		return -3;
	}
	else if (data) {
		*((const char **) data) = base;
		s->_off += len + 1;
		s->_len -= len + 1;
	}
	
	return len;
}

/*!
 * \ingroup mptClient
 * \brief client preparation
 * 
 * Apply solver configuration from arguments in event data.
 * Prepare client for run and display initial output.
 * 
 * \param cl  client descriptor
 * \param ev  event data
 * 
 * \return hint to event controller (int)
 */
extern int mpt_cevent_prep(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	static const MPT_INTERFACE_VPTR(source) dataVptr = { sliceConv };
	struct {
		MPT_INTERFACE(source) base;
		MPT_STRUCT(slice) d;
	} data = { { &dataVptr} , MPT_SLICE_INIT };
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	int ret;
	
	if (!ev) {
		return 0;
	}
	if (!cl) {
		errno = EFAULT;
		return-1;
	}
	/* apply command line client parameters */
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		ssize_t part = 0;
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)
		    || mt.cmd != MPT_ENUM(MessageCommand)) {
			mpt_output_log(cl->out, __func__, MPT_ENUM(LogError) | MPT_ENUM(LogFunction), "%s",
			               MPT_tr("bad message format"));
			return -1;
		}
		else if ((part = mpt_message_argv(&msg, mt.arg)) > 0) {
			uint8_t *dest;
			mpt_message_read(&msg, part, 0);
			/* set client parameters from arguments */
			while ((part = mpt_message_argv(&msg, mt.arg)) > 0) {
				dest = mpt_array_slice(&data.d._a, data.d._len, part+1);
				mpt_message_read(&msg, part, dest);
				dest[part] = 0;
				data.d._len += part+1;
			}
		}
	}
	/* prepare client */
	ret = cl->_vptr->prep(cl, data.d._len ? &data.base : 0);
	mpt_array_clone(&data.d._a, 0);
	
	if (ret < 0) {
		return MPT_event_fail(ev, MPT_tr("client preparation failed"));
	}
	/* initial output */
	if (cl->_vptr->output(cl, MPT_ENUM(OutputStateInit)) < 0) {
		return MPT_event_fail(ev, MPT_tr("client output failed"));
	}
	return MPT_event_good(ev, MPT_tr("client preparation completed"));
}
