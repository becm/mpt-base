/*!
 * initialize solver from shared library.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "meta.h"

#include "object.h"

#include "client.h"

struct _mpt_metaProxy
{
	MPT_INTERFACE(metatype) _mt;
	MPT_STRUCT(libhandle) lh;
	void *ptr;
	int type;
	char fmt[2];
};

static void mpUnref(MPT_INTERFACE(reference) *m)
{
	struct _mpt_metaProxy *mp = (void *) m;
	
	if ((m = mp->ptr)) {
		if (mp->type) {
			m->_vptr->unref(m);
		} else {
			free(m);
		}
	}
	mpt_library_close(&mp->lh);
	free(mp);
}
static uintptr_t mpRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
static int mpConv(const MPT_INTERFACE(metatype) *m, int type, void *ptr)
{
	const struct _mpt_metaProxy *mp = (void *) m;
	
	if (!(m = mp->ptr)) {
		return MPT_ERROR(BadArgument);
	}
	if (type & ~0xff) {
		return MPT_ERROR(BadValue);
	}
	if (!type) {
		if (ptr) *((void **) ptr) = (char *) mp->fmt;
		return mp->type;
	}
	if (type == mp->type) {
		if (ptr) *((void **) ptr) = mp->ptr;
		return mp->fmt[0];
	}
	if (type == MPT_ENUM(TypeObject)) {
		int mt = mp->type;
		if (!MPT_value_isObject(mt)) {
			return MPT_ERROR(BadType);
		}
		if (ptr) *((void **) ptr) = mp->ptr;
		return mt;
	}
	if (mp->type == MPT_ENUM(TypeMeta)) {
		return m->_vptr->conv(m, type, ptr);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *mpClone(const MPT_INTERFACE(metatype) *m)
{
	const struct _mpt_metaProxy *mp = (void *) m;
	struct _mpt_metaProxy *n;
	void *ptr;
	
	if (mp->lh.lib || !mp->lh.create) {
		return 0;
	}
	if (!(ptr = mp->lh.create())) {
		return 0;
	}
	if (!(n = malloc(sizeof(*n)))) {
		return 0;
	}
	*n = *mp;
	n->ptr = ptr;
	n->type = mp->type;
	n->fmt[0] = mp->fmt[0];
	n->fmt[1] = 0;
	
	return &n->_mt;
}
static const MPT_INTERFACE_VPTR(metatype) _mpt_metaProxyCtl = {
	{ mpUnref, mpRef },
	mpConv,
	mpClone
};

/*!
 * \ingroup mptLoader
 * \brief metatype proxy
 * 
 * Create metatype proxy to library instance.
 * 
 * \param lh    library handle data
 * \param type  type of instance to create
 * 
 * \return metatype proxy pointer
 */
extern MPT_INTERFACE(metatype) *mpt_library_meta(const MPT_STRUCT(libhandle) *lh, int type)
{
	struct _mpt_metaProxy *mp;
	
	if (!lh->create) {
		errno = EINVAL;
		return 0;
	}
	if (!(mp = malloc(sizeof(*mp)))) {
		return 0;
	}
	mp->_mt._vptr = &_mpt_metaProxyCtl;
	mp->lh = *lh;
	
	mp->ptr = lh->create();
	mp->type = type;
	
	mp->fmt[0] = (type >= 0 && type <= 0xff) ? type : 0;
	mp->fmt[1] = 0;
	
	return &mp->_mt;
}
