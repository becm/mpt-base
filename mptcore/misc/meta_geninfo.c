/*!
 * basic metatype
 */

#include <errno.h>
#include <stdlib.h>

#include "meta.h"

static void metaUnref(MPT_INTERFACE(reference) *ref)
{
	free(ref);
}
static uintptr_t metaRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
static int metaConv(const MPT_INTERFACE(metatype) *meta, int type, void *ptr)
{
	const void *info = meta + 1;
	void **dest = ptr;
	
	if (!type) {
		static const char types[] = { MPT_ENUM(TypeMeta), 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (dest) *dest = (void *) meta;
		return 0;
	}
	return _mpt_geninfo_conv(info, type, ptr);
}
static MPT_INTERFACE(metatype) *metaClone(const MPT_INTERFACE(metatype) *meta)
{
	return _mpt_geninfo_clone(meta + 1);
}
static const MPT_INTERFACE_VPTR(metatype) _vptr_control = {
	{ metaUnref, metaRef },
	metaConv,
	metaClone
};

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
