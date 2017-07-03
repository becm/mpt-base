/*!
 * change input for metatype
 */

#include <inttypes.h>

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
extern int mpt_notify_change(MPT_STRUCT(notify) *no, MPT_INTERFACE(metatype) *mt, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(input) *in, *old, **slot;
	MPT_INTERFACE(object) *obj;
	MPT_STRUCT(buffer) *buf;
	int ret, fd = -1;
	
	if (!(buf = no->_slot._buf)) {
		return MPT_ERROR(BadArgument);
	}
	in = 0;
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeInput), &in)) < 0
	    || !in) {
		return MPT_ERROR(BadArgument);
	}
	obj = 0;
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj)) < 0
	    || !obj) {
		return MPT_ERROR(BadArgument);
	}
	if ((fd = in->_vptr->_file(in)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	slot = (void *) (buf + 1);
	
	if ((buf->_used / sizeof(*slot)) < (size_t) fd
	    || in != slot[fd]) {
		return MPT_ERROR(BadValue);
	}
	if (!val) {
		ret = obj->_vptr->setProperty(obj, 0, 0);
	} else if (!val->fmt) {
		ret = mpt_object_pset(obj, 0, val->ptr, 0);
	} else {
		MPT_STRUCT(value) tmp = *val;
		ret = mpt_object_nset(obj, 0, &tmp);
	}
	if (ret < 0) {
		return ret;
	}
	if ((ret = in->_vptr->_file(in)) < 0) {
		mpt_notify_clear(no, fd);
		return 0;
	}
	/* same descriptor */
	if (ret == fd) {
		return 0;
	}
	/* unable to reserve new space */
	if (!(slot = mpt_array_slice(&no->_slot, ret * sizeof(*slot), sizeof(*slot)))) {
		mpt_notify_clear(no, fd);
		return MPT_ERROR(BadOperation);
	}
	/* descriptor already removed */
	if (!(old = slot[fd])) {
		return 0;
	}
	/* target has open descriptor */
	if ((old = slot[ret])) {
		mpt_log(0, __func__, MPT_LOG(Warning), "%s: " PRIxPTR "-> %d",
		        MPT_tr("replace active input slot"), no, ret);
		old->_vptr->ref.unref((void *) old);
	}
#if defined(__linux)
	if (no->_sysfd >= 0) {
		struct epoll_event ev;
		int err;
		
		ev.data.fd = fd;
		ev.events = 0;
		
		if ((err = epoll_ctl(no->_sysfd, EPOLL_CTL_DEL, fd, &ev)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Warning), "%s: " PRIxPTR "-> %d",
			        MPT_tr("unable to remove active epoll descriptor"), no, fd);
		}
		ev.data.fd = ret;
		ev.events = EPOLLIN;
		if ((err = epoll_ctl(no->_sysfd, EPOLL_CTL_ADD, ret, &ev)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Warning), "%s: " PRIxPTR "-> %d",
			        MPT_tr("unable to set new epoll descriptor"), no, ret);
		}
	}
#endif
	slot[ret] = in;
	slot[fd] = 0;
	
	return 2;
}
