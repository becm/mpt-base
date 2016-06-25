/*!
 * default metatype
 */

#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "array.h"

#include "meta.h"

static void metaUnref(MPT_INTERFACE(metatype) *meta)
{
	free(meta);
}
static int metaAssign(MPT_INTERFACE(metatype) *meta, const MPT_STRUCT(value) *val)
{
	uint64_t *info = (uint64_t *) (meta + 1);
	if (val) {
		return _mpt_geninfo_value(info, val);
	}
	return 0;
}
static int metaConv(MPT_INTERFACE(metatype) *meta, int type, void *ptr)
{
	uint64_t *info = (uint64_t *) (meta + 1);
	void **dest = ptr;
	
	if (type & MPT_ENUM(ValueConsume)) {
		return MPT_ERROR(BadArgument);
	}
	if (!type) {
		static const char types[] = { MPT_ENUM(TypeMeta), 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	switch (type & 0xff) {
	  case MPT_ENUM(TypeMeta): ptr = meta; break;
	  default: return _mpt_geninfo_conv(info, type, ptr);
	}
	if (dest) *dest = ptr;
	return type & 0xff;
}
static MPT_INTERFACE(metatype) *metaClone(const MPT_INTERFACE(metatype) *meta)
{
	uint64_t *info = (uint64_t *) (meta + 1);
	return _mpt_geninfo_clone(info);
}
static const MPT_INTERFACE_VPTR(metatype) _vptr_control = {
	metaUnref,
	metaAssign,
	metaConv,
	metaClone
};

/*!
 * \ingroup mptMeta
 * \brief new metatype
 * 
 * Create default metatype
 * 
 * \param post  additional size for data
 * 
 * \return new metatype instance
 */
extern MPT_INTERFACE(metatype) *mpt_meta_new(size_t post)
{
	MPT_INTERFACE(metatype) *meta;
	uint64_t *info;
        static const size_t min = (32 - sizeof(info) - sizeof(*meta));
	
	if (post < min) {
		post = min;
	}
	else if (post > UINT8_MAX) {
		MPT_STRUCT(array) a = MPT_ARRAY_INIT;
		
		if (!mpt_array_slice(&a, 0, post)) {
			return 0;
		}
		meta = mpt_meta_buffer(&a);
		mpt_array_clone(&a, 0);
		return meta;
	}
	else {
		post = MPT_align(post);
	}
	post += sizeof(*info);
	
	if (!(meta = malloc(sizeof(*meta)+post))) {
		return 0;
	}
	info = (void *) (meta+1);
	
	/* initial settings */
	if (_mpt_geninfo_init(info, post) < 0) {
		free(meta);
		return 0;
	}
	meta->_vptr = &_vptr_control;
	
	return meta;
}
