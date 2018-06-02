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

#include "notify.h"

#if defined(__linux__)
# include <sys/epoll.h>
#endif

static int _input_ref_type = 0;

static void _input_ref_init(const MPT_STRUCT(type_traits) *info, void *ptr)
{
	memset(ptr, 0, info->size);
}
static int _input_ref_copy(void *src, int type, void *dest)
{
	MPT_INTERFACE(reference) **src_ptr, *src_ref, **dest_ptr, *dest_ref;
	
	src_ptr = src;
	dest_ptr = dest;
	
	if (type != MPT_ENUM(TypeMeta)) {
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
	MPT_INTERFACE(reference) **ref_ptr, *ref;
	
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
	int file;
	
	if (!_input_ref_type) {
		_input_ref_type = mpt_input_typeid();
	}
	if (_input_ref_type < 0) {
		return MPT_ERROR(BadEncoding);
	}
	
	file = -1;
	if (!in
	    || in->_vptr->meta.conv((void *) in, MPT_ENUM(TypeSocket), &file) < 0
	    || file < 0) {
		return MPT_ERROR(BadArgument);
	}
	/* create new buffer */
	if (!(buf = no->_slot._buf)) {
		MPT_STRUCT(type_traits) info = MPT_TYPETRAIT_INIT(*base, MPT_ENUM(TypeMeta));
		size_t len;
		
		info.init = _input_ref_init;
		info.copy = _input_ref_copy;
		info.fini = _input_ref_fini;
		
		info.type = _input_ref_type;
		info.size = sizeof(*base);
		info.base = MPT_ENUM(TypeMeta);
		
		len = (file + 1) * sizeof(*base);
		if (!(buf = _mpt_buffer_alloc(len, &info))) {
			return 0;
		}
		base = memset(buf + 1, 0, len);
		base += file;
	}
	/* reject incompatible data */
	else if (!(info = buf->_typeinfo) || info->type != _input_ref_type) {
		return MPT_ERROR(BadType);
	}
	/* reserve matching position */
	else {
		if (!(base = mpt_array_slice(&no->_slot, file, sizeof(*base)))) {
			return MPT_ERROR(BadOperation);
		}
		if (*base) {
			return MPT_ERROR(BadArgument);
		}
	}
#if defined(__linux__)
	if (no->_sysfd < 0 && !no->_fdused) {
		const MPT_INTERFACE(metatype) *mt;
		const char *val = 0;
		int32_t flg = 1;
		if (!(mt = mpt_config_get(0, "mpt.notify.epoll", '.', 0))) {
			no->_sysfd = epoll_create1(EPOLL_CLOEXEC);
		}
		else if (mt->_vptr->conv(mt, 'i', &flg) > 0) {
			if (flg) no->_sysfd = epoll_create1(EPOLL_CLOEXEC);
		}
		else if (mt->_vptr->conv(mt, 's', &val) < 0
		         || !val
		         || strcmp(val, "false")) {
			no->_sysfd = epoll_create1(EPOLL_CLOEXEC);
		}
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
		curr->_vptr->meta.ref.unref((void *) curr);
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
