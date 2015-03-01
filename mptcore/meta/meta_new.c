/*!
 * default metatype
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "array.h"

#define MPT_META_POST	8

static int metaUnref(MPT_INTERFACE(metatype) *meta)
{
	uint32_t c = _mpt_geninfo_unref((uint64_t *) (meta+1));
	if (!c) free(meta);
	return c;
}
static MPT_INTERFACE(metatype) *metaAddref(MPT_INTERFACE(metatype) *meta)
{ return _mpt_geninfo_addref((uint64_t *) (meta+1)) ? meta : 0; }

static int metaProperty(MPT_INTERFACE(metatype) *meta, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	uint64_t *info = (uint64_t *) (meta + 1);
	if (src || prop) {
		return _mpt_geninfo_property(info, prop, src);
	}
	return 0;
}
static void *metaCast(MPT_INTERFACE(metatype) *meta, int type)
{
	uint64_t *info = (uint64_t *) (meta + 1);
	switch (type) {
	  case MPT_ENUM(TypeMeta): return meta;
	  case 's': return _mpt_geninfo_property(info, 0, 0) > 0 ? (info + 1) : 0;
	  default: return 0;
	}
}
static const MPT_INTERFACE_VPTR(metatype) _vptr_control = {
	metaUnref,
	metaAddref,
	metaProperty,
	metaCast
};

/*!
 * \ingroup mptMeta
 * \brief new metatype
 * 
 * Create default metatype
 * 
 * \param post	additional size for data
 * 
 * \return new metatype instance
 */
extern MPT_INTERFACE(metatype) *mpt_meta_new(size_t post)
{
	MPT_INTERFACE(metatype) *meta;
	uint64_t *info;
	
	if (post < MPT_META_POST) {
		post = MPT_META_POST;
	}
	else if (post > UINT8_MAX) {
		return mpt_meta_buffer(post, 0);
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
	if (_mpt_geninfo_init(info, post, 1) < 0) {
		free(meta);
		return 0;
	}
	meta->_vptr = &_vptr_control;
	
	return meta;
}
