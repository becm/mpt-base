/*!
 * add/remove input slot
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "meta.h"
#include "array.h"
#include "config.h"

#include "connection.h"

#include "notify.h"

#if defined(__linux__)
# include <sys/epoll.h>
#endif

static int _input_type = 0;

static void _input_ref_init(const MPT_STRUCT(type_traits) *info, void *ptr)
{
	memset(ptr, 0, info->size);
}
static int _input_ref_copy(void *src, int type, void *dest)
{
	MPT_INTERFACE(metatype) **src_ptr, *src_ref, **dest_ptr, *dest_ref;
	
	src_ptr = src;
	dest_ptr = dest;
	
	if (type != MPT_type_reference(_input_type)) {
		return MPT_ERROR(BadType);
	}
	if ((src_ref = *src_ptr)
	    && !(src_ref->_vptr->addref(src_ref))) {
		return MPT_ERROR(BadOperation);
	}
	if ((dest_ref = *dest_ptr)) {
		dest_ref->_vptr->unref(dest_ref);
	}
	*dest_ptr = src_ref;
	return src_ref ? 1 : 0;
}
static void _input_ref_fini(void *ptr)
{
	MPT_INTERFACE(metatype) **ref_ptr, *ref;
	
	ref_ptr = ptr;
	if ((ref = *ref_ptr)) {
		ref->_vptr->unref(ref);
		*ref_ptr = 0;
	}
}

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
	const MPT_STRUCT(type_traits) *info;
	MPT_STRUCT(buffer) *buf;
	MPT_INTERFACE(input) **base;
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	
	if (!_input_type) {
		_input_type = mpt_input_typeid();
	}
	if (_input_type < 0) {
		return MPT_ERROR(BadEncoding);
	}
	if (!in
	    || in->_vptr->meta.convertable.convert((void *) in, MPT_ENUM(TypeSocket), &sock) < 0
	    || !MPT_socket_active(&sock)) {
		return MPT_ERROR(BadArgument);
	}
	/* create new buffer */
	if (!(buf = no->_slot._buf)) {
		MPT_STRUCT(type_traits) info;
		size_t len;
		
		memset(&info, 0, sizeof(info));
		info.init = _input_ref_init;
		info.copy = _input_ref_copy;
		info.fini = _input_ref_fini;
		
		info.type = MPT_type_reference(_input_type);
		info.size = sizeof(*base);
		info.base = MPT_ENUM(TypeMetaRef);
		
		len = (sock._id + 1) * sizeof(*base);
		if (!(buf = _mpt_buffer_alloc(len, &info))) {
			return 0;
		}
		buf->_used = len;
		base = memset(buf + 1, 0, len);
		base += sock._id;
		
		no->_slot._buf = buf;
	}
	/* reject incompatible data */
	else if (!(info = buf->_typeinfo) || info->type != MPT_type_reference(_input_type)) {
		return MPT_ERROR(BadType);
	}
	/* reserve matching position */
	else {
		if (!(base = mpt_array_slice(&no->_slot, sock._id * sizeof(*base), sizeof(*base)))) {
			return MPT_ERROR(BadOperation);
		}
		if (*base) {
			return MPT_ERROR(BadArgument);
		}
	}
#if defined(__linux__)
	if (no->_sysfd < 0 && !no->_fdused) {
		MPT_INTERFACE(convertable) *val;
		const char *choice = 0;
		int32_t flg = 1;
		if (!(val = mpt_config_get(0, "mpt.notify.epoll", '.', 0))) {
			no->_sysfd = epoll_create1(EPOLL_CLOEXEC);
		}
		else if (val->_vptr->convert(val, 'i', &flg) > 0) {
			if (flg) no->_sysfd = epoll_create1(EPOLL_CLOEXEC);
		}
		else if (val->_vptr->convert(val, 's', &choice) < 0
		         || !choice
		         || strcmp(choice, "false")) {
			no->_sysfd = epoll_create1(EPOLL_CLOEXEC);
		}
	}
	if (no->_sysfd >= 0) {
		struct epoll_event ev;
		ev.data.fd = sock._id;
		ev.events  = mode;
		if ((mode = epoll_ctl(no->_sysfd, EPOLL_CTL_ADD, sock._id, &ev)) < 0) {
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
		curr->_vptr->meta.unref((void *) curr);
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
