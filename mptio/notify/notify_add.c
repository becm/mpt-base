/*!
 * add/remove input slot
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "array.h"

#include "notify.h"

#if defined(__linux__)
# include <sys/epoll.h>
#endif

static void notifyDataUnref(MPT_INTERFACE(unrefable) *ref)
{
	MPT_STRUCT(buffer) *buf = (void *) ref;
	
	if (!mpt_reference_lower(&buf->_ref)) {
		MPT_INTERFACE(input) **base = (void *) (buf + 1);
		size_t i, len = buf->_used / sizeof(*base);
		for (i = 0; i < len; ++i) {
			MPT_INTERFACE(input) *curr;
			if ((curr = base[i])) {
				curr->_vptr->ref.unref((void *) curr);
			}
		}
		free(buf);
	}
}
static MPT_STRUCT(buffer) *notifyDataDetach(MPT_STRUCT(buffer) *buf, long len)
{
	/* unable to clone input references */
	if (buf->_ref._val > 1) {
		errno = ENOTSUP;
		return 0;
	}
	len *= sizeof(void *);
	if (len < (long) (buf->_used)) {
		return buf;
	}
	if ((buf = realloc(buf, sizeof(*buf) + len))) {
		buf->_size = len;
	}
	return buf;
}
static int notifyDataType(const MPT_STRUCT(buffer) *buf)
{
	(void) buf;
	return MPT_ENUM(TypeInput);
}
static MPT_INTERFACE_VPTR(buffer) notifyDataCtl = {
	{ notifyDataUnref },
	notifyDataDetach,
	notifyDataType
};

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
	MPT_STRUCT(buffer) *buf;
	MPT_INTERFACE(input) **base;
	int file;
	
	if (!in || (file = in->_vptr->_file(in)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	if ((buf = no->_slot._buf)) {
		if (buf->_vptr->content(buf) != notifyDataType(0)) {
			buf->_vptr->ref.unref((void *) buf);
			no->_slot._buf = buf = 0;
		}
	}
	if (!buf) {
		size_t len = (file + 1) * sizeof(*base);
		if (!(buf = malloc(sizeof(*buf) + len))) {
			return 0;
		}
		buf->_vptr = &notifyDataCtl;
		buf->_ref._val = 1;
		buf->_size = len;
		buf->_used = 0;
		no->_slot._buf = buf;
	}
	/* check for existing */
	else if ((base = mpt_buffer_data(buf, file, sizeof(*base)))
	       && *base) {
		errno = EADDRINUSE;
		return -2;
	}
	else {
		long len = buf->_used / sizeof(*base);
		if (len <= file) {
			len = file + 1;
		}
		if (!(buf = buf->_vptr->detach(buf, len))) {
			return 0;
		}
		no->_slot._buf = buf;
	}
	/* add new element */
	if ((base = mpt_buffer_insert(buf, file * sizeof(*base), sizeof(*base)))) {
		*base = 0;
	} else {
		return -1;
	}
#if defined(__linux__)
	if (no->_sysfd < 0 && !no->_fdused) {
		no->_sysfd = epoll_create1(EPOLL_CLOEXEC);
	}
	if (no->_sysfd >= 0) {
		struct epoll_event ev;
		ev.data.fd = file;
		ev.events  = mode;
		if ((mode = epoll_ctl(no->_sysfd, EPOLL_CTL_ADD, file, &ev)) < 0) {
			return mode;
		}
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
	MPT_STRUCT(buffer) *b;
	
	if ((b = no->_slot._buf)
	 && (base = mpt_buffer_data(b, file, sizeof(*base)))) {
		MPT_INTERFACE(input) *curr;
		MPT_STRUCT(buffer) *buf;
		if (!(curr = *base)) {
			errno = EBADF;
			return -2;
		}
		curr->_vptr->ref.unref((void *) curr);
		*base = 0;
		if ((buf = no->_wait._buf)) {
			size_t i, len = buf->_used / sizeof(*base);
			for (i = 0; i < len; ++i) {
				if (base[i] == curr) {
					base[i] = 0;
				}
			}
		}
		--no->_fdused;
	}
#if defined(__linux)
	if (no->_sysfd >= 0) {
		struct epoll_event ev;
		
		ev.data.fd = file;
		ev.events  = 0;
		
		if ((file = epoll_ctl(no->_sysfd, EPOLL_CTL_DEL, file, &ev)) < 0) {
			return file;
		}
	}
#endif
	return no->_fdused;
}
