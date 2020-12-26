/*!
 * basic metatype
 */

#include <errno.h>
#include <stdlib.h>

#include "types.h"

#include "meta.h"

/* metatype interface */
static int metaConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	const void *info = val + 1;
	void **dest = ptr;
	
	if (!type) {
		static const char types[] = { 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (dest) *dest = (void *) val;
		return 0;
	}
	return _mpt_geninfo_conv(info, type, ptr);
}
static void metaUnref(MPT_INTERFACE(metatype) *in)
{
	free(in);
}
static uintptr_t metaRef(MPT_INTERFACE(metatype) *in)
{
	(void) in;
	return 0;
}
static MPT_INTERFACE(metatype) *metaClone(const MPT_INTERFACE(metatype) *mt)
{
	return _mpt_geninfo_clone(mt + 1);
}

/*!
 * \ingroup mptMeta
 * \brief new basic metatype
 * 
 * Create basict small text metatype
 * 
 * \param src  text data (255 chars max)
 * \param len  text length (-1 to detect)
 * 
 * \return new basic metatype instance
 */
extern MPT_INTERFACE(metatype) *mpt_meta_geninfo(size_t post)
{
	static const MPT_INTERFACE_VPTR(metatype) _vptr_control = {
		{ metaConv },
		metaUnref,
		metaRef,
		metaClone
	};
	MPT_INTERFACE(metatype) *mt;
	
	post = _mpt_geninfo_size(post + 1);
	if (post > UINT8_MAX) {
		errno = EINVAL;
		return 0;
	}
	if (!(mt = malloc(sizeof(*mt) + post))) {
		return 0;
	}
	/* initial settings */
	if (_mpt_geninfo_init(mt + 1, post) < 0) {
		free(mt);
		return 0;
	}
	mt->_vptr = &_vptr_control;
	
	return mt;
}
