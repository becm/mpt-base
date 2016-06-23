/*!
 * initialize solver from shared library.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

struct _mpt_metaProxy
{
	MPT_INTERFACE(metatype) _mt;
	MPT_STRUCT(libhandle) lh;
	void *ptr;
	int type;
	char fmt[2];
};

static void mpUnref(MPT_INTERFACE(metatype) *m)
{
	struct _mpt_metaProxy *mp = (void *) m;
	
	if ((m = mp->ptr)) {
		if (mp->type) {
			m->_vptr->unref(m);
		}
		else if (!mp->type) {
			free(m);
		}
	}
	mpt_library_close(&mp->lh);
	free(mp);
}
static int mpAssign(MPT_INTERFACE(metatype) *m, const MPT_STRUCT(value) *val)
{
	struct _mpt_metaProxy *mp = (void *) m;
	int type;
	
	if (!(m = mp->ptr)
	    || !(mp->ptr = m = mp->lh.create())) {
		return MPT_ERROR(BadArgument);
	}
	type = mp->type;
	/* assign metatype */
	if (type == MPT_ENUM(TypeMeta)) {
		return m->_vptr->assign(m, val);
	}
	/* set generic object property */
	if (MPT_value_isObject(type)) {
		return mpt_object_pset(mp->ptr, 0, val, 0);
	}
	return MPT_ERROR(BadType);
}
static int mpConv(MPT_INTERFACE(metatype) *m, int type, void *ptr)
{
	struct _mpt_metaProxy *mp = (void *) m;
	
	if (!(m = mp->ptr)
	    || !(mp->ptr = m = mp->lh.create())) {
		return MPT_ERROR(BadArgument);
	}
	if (type & ~0xff) {
		return MPT_ERROR(BadValue);
	}
	if (!type) {
		if (ptr) *((void **) ptr) = mp->fmt;
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
static MPT_INTERFACE(metatype) *mpClone(MPT_INTERFACE(metatype) *m)
{
	(void) m;
	return 0;
}
static const MPT_INTERFACE_VPTR(metatype) _mpt_metaProxyCtl = {
	mpUnref,
	mpAssign,
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
	
	if (!(mp = malloc(sizeof(*mp)))) {
		return 0;
	}
	mp->_mt._vptr = &_mpt_metaProxyCtl;
	mp->lh = *lh;
	
	mp->ptr = 0;
	mp->type = type;
	
	mp->fmt[0] = (type >= 0 && type <= 0xff) ? type : 0;
	mp->fmt[1] = 0;
	
	return &mp->_mt;
}
