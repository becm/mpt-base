/*!
 * wait for IO via system-poll-descriptor or generic poll.
 */

#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include <poll.h>

#if defined(__linux)
# include <sys/epoll.h>
#endif

#include "array.h"
#include "event.h"

#include "notify.h"

/*!
 * \ingroup mptNotify
 * \brief wait for available operation
 * 
 * Check registered inputs and outputs for available actions.
 * Process (part of) available data.
 * 
 * \param no	notification descriptor
 * \param what	type of wait operations
 * \param timeout wait time in milliseconds
 */
extern int mpt_notify_wait(MPT_STRUCT(notify) *no, int what, int timeout)
{
	struct pollfd	 *ev;
	MPT_STRUCT(buffer) *buf;
	MPT_INTERFACE(input) *curr, **pslot, **slot;
	size_t	fdmax, used;
	int i, ret, act;
	
	if (!no) {
		errno = EFAULT; return -1;
	}
	if (!(ret = no->_fdused)) {
		errno = EAGAIN; return -2;
	}
	pslot = (void *) (no->_slot._buf + 1);
	used  = no->_wait._buf ? no->_wait._buf->used : 0;
	
#if defined(__linux)
	if (no->_sysfd >= 0) {
		struct epoll_event *epv;
		
		fdmax = ret * sizeof(*epv);
		if (!(epv = mpt_array_slice(&no->_wait, 0, fdmax))) {
			return -1;
		}
		buf = no->_wait._buf;
		
		if ((act = epoll_wait(no->_sysfd, epv, ret, timeout)) <= 0) {
			if (act < 0 && errno == EINTR) {
				act = 0;
			}
			return (buf->used = used) ? (int) (used / sizeof(curr)) : act;
		}
		slot = (MPT_INTERFACE(input) **) epv;
		
		for (i = 0, fdmax = 0; i < act; i++) {
			if (!(ret = what & epv[i].events))
				continue;
			
			if (!(curr = pslot[epv[i].data.fd]))
				continue;
			
			/* call selector */
			if ((ret = curr->_vptr->next(curr, ret)) < 0) {
				mpt_notify_clear(no, epv[i].data.fd);
				continue;
			}
			if (ret) slot[fdmax++] = curr;
		}
		buf->used = fdmax * sizeof(void*);
		
		return fdmax;
	}
#endif
	fdmax = ret * sizeof(*ev);
	if (!(ev = mpt_array_slice(&no->_wait, 0, fdmax))) {
		return -1;
	}
	buf = no->_wait._buf;
	ret = no->_slot._buf->used / sizeof(curr);
	
	for (i = 0, act = 0; i < ret; i++) {
		if (!pslot[i] )
			continue;
		
		if (act >= ret)
			break;
		
		if (!(ev[act].events = ~what))
			continue;
		
		ev[act].fd = i;
		ev[act++].revents = 0;
	}
	
	if ((act = poll(ev, act, timeout)) <= 0) {
		return (buf->used = used) ? (int) (used / sizeof(curr)) : act;
	}
	slot = (MPT_INTERFACE(input) **) ev;
	
	for (i = 0, fdmax = 0; i < act; i++) {
		if (!(ret = ev[i].revents & what))
			continue;
		
		if (!(curr = slot[ev[i].fd]))
			continue;
		
		/* call selector */
		if ((ret = curr->_vptr->next(curr, ret)) < 0) {
			mpt_notify_clear(no, ev[i].fd);
			continue;
		}
		if (ret) slot[fdmax++] = curr;
	}
	buf->used = fdmax * sizeof(void*);
	
	return fdmax;
}
