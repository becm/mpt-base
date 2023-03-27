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
int metatype::convert(value_t type, void *ptr)
{
	void **dest = (void **) ptr;
	
	/* identify as metatype */
	if (!type) {
		static const uint8_t types[] = { TypeConvertablePtr, 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	/* support (up-)cast to metatype pointer */
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
int metatype::basic::convert(value_t type, void *ptr)
{
	if (!type) {
		metatype::convert(type, ptr);
		return _mpt_geninfo_conv(this + 1, 0, 0);
	}
	int me = mpt::type_properties<basic *>::id(true);
	if (type == TypeMetaPtr) {
		if (ptr) {
			*static_cast<metatype **>(ptr) = this;
		}
		return me > 0 ? me : static_cast<int>(TypeMetaPtr);
	}
	if (me > 0 && type == static_cast<value_t>(me)) {
		if (ptr) {
			*static_cast<basic **>(ptr) = this;
		}
		return static_cast<int>(TypeMetaPtr);
	}
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

/*!
 * \ingroup mptMeta
 * \brief query traits for basic metatype
 * 
 * Query and register named traits for specialized metatype pointer.
 * 
 * \param val  initial metatype value
 * 
 * \return new metatype
 */
const struct named_traits *metatype::basic::pointer_traits(bool obtain)
{
	static const struct named_traits *traits = 0;
	if (!traits && obtain && !(traits = type_traits::add_metatype("basic"))) {
		traits = type_traits::add_metatype();
	}
	return traits;
}

__MPT_NAMESPACE_END
