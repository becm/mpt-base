/*!
 * change input for metatype
 */

#include <inttypes.h>
#include <poll.h>

#if defined(__linux__)
# include <sys/epoll.h>
#endif

#include "meta.h"
#include "object.h"

#include "notify.h"

/*!
 * \ingroup mptOutput
 * \brief input change filter
 * 
 * Update notifier on associated descriptor change.
 * 
 * Add, remove or replace input on new and old file positions.
 * 
 * \param no  notification descriptor
 * \param mt  input metatype to change
 * \param val vew value for input
 * 
 * \return output descriptor
 */
extern int mpt_notify_change(MPT_STRUCT(notify) *no, MPT_INTERFACE(input) *next, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(input) *in, **slot;
	MPT_INTERFACE(object) *obj;
	MPT_STRUCT(buffer) *buf;
	int ret, fdold, fdnew, len;
	
	fdold = -1;
	if ((ret = next->_vptr->meta.conv((void *) next, MPT_ENUM(TypeSocket), &fdold)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	obj = 0;
	if ((ret = next->_vptr->meta.conv((void *) next, MPT_ENUM(TypeObject), &obj)) < 0
	    || !obj) {
		return MPT_ERROR(BadArgument);
	}
	/* get old slot pointer */
	in = 0;
	len = 0;
	slot = 0;
	if (fdold >= 0
	    && (buf = no->_slot._buf)
	    && (len = buf->_used / sizeof(*slot))
	    && len > fdold) {
		slot = (void *) (buf + 1);
		in = slot[fdold];
	}
	/* assign new target */
	if (!val) {
		ret = obj->_vptr->set_property(obj, 0, 0);
	} else if (!val->fmt) {
		ret = mpt_object_set_string(obj, 0, val->ptr, 0);
	} else {
		MPT_STRUCT(value) tmp = *val;
		ret = mpt_object_set_value(obj, 0, &tmp);
	}
	/* get new descriptor */
	fdnew = -1;
	next->_vptr->meta.conv((void *) next, MPT_ENUM(TypeSocket), &fdnew);
	
	/* distinct instances */
	if (next != in) {
		if (fdnew < 0) {
			return 0;
		}
		/* need separate reference */
		if (!(next->_vptr->meta.instance.addref((void *) next))) {
			return MPT_ERROR(BadOperation);
		}
	}
	/* no change in position */
	else if (fdnew == fdold) {
		return 0;
	}
	/* detach reference from old slot */
	else if (in) {
		slot[fdold] = 0;
		if (fdold >= 0) {
			mpt_notify_clear(no, fdold);
		}
	}
	/* move reference to notifier */
	if (mpt_notify_add(no, POLLIN, next) < 0) {
		in->_vptr->meta.instance.unref((void *) in);
		return MPT_ERROR(BadOperation);
	}
	
	return fdold < 0 ? 1 : 2;
}
