/*!
 * add/remove input slot
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "array.h"
#include "event.h"

#include "notify.h"

#if defined(__linux)
# include <sys/epoll.h>
#endif

/*!
 * \ingroup mptNotify
 * \brief add input to notifier
 * 
 * Add input reference to notifier in specified poll mode.
 * Notifier assumes ownership of passed reference.
 * 
 * \param no   notification descriptor
 * \param mode type of events to listen for
 * \param in   input reference
 */
extern int mpt_notify_add(MPT_STRUCT(notify) *no, int mode, MPT_INTERFACE(input) *in)
{
	MPT_INTERFACE(input) **base;
	int file;
	
	if (!in || (file = in->_vptr->_file(in)) < 0) {
		errno = EBADF; return -1;
	}
	/* check for existing */
	if ((base = mpt_array_data(&no->_slot, file, sizeof(*base)))) {
		if (*base) {
			errno = EADDRINUSE;
			return -2;
		}
	}
	/* add new element */
	else if ((base = mpt_array_insert(&no->_slot, file*sizeof(*base), sizeof(*base)))) {
		*base = 0;
	}
	else {
		return -1;
	}
#if defined(__linux)
	if (no->_sysfd < 0 && !no->_fdused) {
		no->_sysfd = epoll_create1(EPOLL_CLOEXEC);
	}
	if (no->_sysfd >= 0) {
		struct epoll_event ev;
		ev.data.fd = file;
		ev.events  = mode;
		if ((mode = epoll_ctl(no->_sysfd, EPOLL_CTL_ADD, file, &ev)) < 0)
			return mode;
	}
#endif
	*base = in;
	++no->_fdused;
	
	return mode;
}


/*!
 * \ingroup mptNotify
 * \brief add input to notifier
 * 
 * Clear input on file descriptor from notifier.
 * Remove reference to input and clear from wait buffer.
 * 
 * \param no   notification descriptor
 * \param file unix file descriptor
 */
extern int mpt_notify_clear(MPT_STRUCT(notify) *no, int file)
{
	MPT_INTERFACE(input) **base;
	
	if ((base = mpt_array_data(&no->_slot, file, sizeof(*base)))) {
		MPT_INTERFACE(input) *curr;
		MPT_STRUCT(buffer) *buf;
		if (!(curr = *base)) {
			errno = EBADF;
			return -2;
		}
		curr->_vptr->unref(curr);
		*base = 0;
		if ((buf = no->_wait._buf)) {
			size_t i, len = buf->used / sizeof(*base);
			for (i = 0; i < len; ++i) {
				if (base[i] == curr) {
					base[i] = 0;
				}
			}
		}
		file = --no->_fdused;
	}
#if defined(__linux)
	if (no->_sysfd >= 0) {
		struct epoll_event ev;
		
		ev.data.fd = file;
		ev.events  = 0;
		
		file = epoll_ctl(no->_sysfd, EPOLL_CTL_DEL, file, &ev);
	}
#endif
	return file;
}
