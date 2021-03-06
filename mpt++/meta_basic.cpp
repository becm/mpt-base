/*
 * MPT C++ metatype creator
 */

#include <limits>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "types.h"

#include "meta.h"

__MPT_NAMESPACE_BEGIN

// default conversion
int metatype::convert(int type, void *ptr)
{
	void **dest = (void **) ptr;
	
	if (!type) {
		static const char types[] = { 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	if (assign(this, type, ptr)) {
		return type;
	}
	return BadType;
}

// basic metatype
metatype::basic::basic(size_t post)
{
	_mpt_geninfo_init(this + 1, post);
}
void metatype::basic::unref()
{
	free(this);
}
int metatype::basic::convert(int type, void *ptr)
{
	return _mpt_geninfo_conv(this + 1, type, ptr);
}
metatype::basic *metatype::basic::clone() const
{
	struct iovec vec;
	if (_mpt_geninfo_conv(this + 1, MPT_type_toVector('c') , &vec) <= 0) {
		errno = EINVAL;
		return 0;
	}
	return create(static_cast<const char *>(vec.iov_base), vec.iov_len);
}
bool metatype::basic::set(const char *src, int len)
{
	return _mpt_geninfo_set(this + 1, src, len) >= 0;
}

/*!
 * \ingroup mptMeta
 * \brief create basic metatype
 * 
 * Create and initialize generic, buffer or special metatype.
 * 
 * \param val  initial metatype value
 * 
 * \return new metatype
 */
metatype::basic *metatype::basic::create(const char *src, int len)
{
	if (len < 0) {
		len = src ? strlen(src) : 0;
	}
	int post;
	if ((post = _mpt_geninfo_size(len)) < 0) {
		errno = EINVAL;
		return 0;
	}
	basic *m;
	void *ptr;
	if (!(ptr = malloc(sizeof(*m) + post))) {
		return 0;
	}
	m = new (ptr) basic(post);
	m->set(src, len);
	return m;
}

__MPT_NAMESPACE_END
