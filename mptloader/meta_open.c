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
	MPT_STRUCT(libhandle)    lh;
	MPT_STRUCT(proxy)        px;
};

static void mpUnref(MPT_INTERFACE(metatype) *m)
{
	struct _mpt_metaProxy *mp = (void *) m;
	
	if ((m = mp->px._ref)) {
		m->_vptr->unref(m);
	}
	mpt_library_close(&mp->lh);
	free(mp);
}
static int mpAssign(MPT_INTERFACE(metatype) *m, const MPT_STRUCT(value) *val)
{
	static const char fmt[] = { MPT_ENUM(TypeValue), 0 };
	struct _mpt_metaProxy *mp = (void *) m;
	MPT_INTERFACE(object) *obj;
	
	if (!(m = mp->px._ref)) {
		return MPT_ERROR(BadArgument);
	}
	/* update vompatible types */
	if (val && val->fmt && !val->ptr) {
		size_t len;
		if (*mp->px._types) {
			return MPT_ERROR(BadOperation);
		}
		len = strlen(val->fmt);
		if (len >= sizeof(mp->px._types)) {
			len = sizeof(mp->px._types) - 1;
		}
		memcpy(mp->px._types, val->fmt, len);
		memset(mp->px._types+len, 0, sizeof(mp->px._types) - len);
		return len;
	}
	if (strchr(mp->px._types, MPT_ENUM(TypeMeta))) {
		return m->_vptr->assign(m, val);
	}
	if (strchr(mp->px._types, MPT_ENUM(TypeObject))) {
		return MPT_ERROR(BadOperation);
	}
	obj = mp->px._ref;
	if (!val) {
		return obj->_vptr->setProperty(obj, 0, 0);
	}
	return mpt_object_set(obj, 0, fmt, *val);
}
static int mpConv(MPT_INTERFACE(metatype) *m, int type, void *ptr)
{
	struct _mpt_metaProxy *mp = (void *) m;
	
	if (!(m = mp->px._ref)) {
		return MPT_ERROR(BadArgument);
	}
	if (type & ~0xff) {
		return MPT_ERROR(BadValue);
	}
	if (!type) {
		if (ptr) *((void **) ptr) = mp->px._types;
		return *mp->px._types;
	}
	if (strchr(mp->px._types, type)) {
		if (ptr) *((void **) ptr) = mp->px._ref;
		return type;
	}
	if (!strchr(mp->px._types, MPT_ENUM(TypeMeta))) {
		return MPT_ERROR(BadType);
	}
	return m->_vptr->conv(m, type, ptr);
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
 * \param descr symbol @ library to open
 * \param path  additional library search path
 * \param out   logging instance
 * 
 * \return metatype proxy pointer
 */
extern MPT_INTERFACE(metatype) *mpt_meta_open(const char *descr, const char *path, MPT_INTERFACE(logger) *out)
{
	struct _mpt_metaProxy *mp;
	MPT_INTERFACE(metatype) *m;
	
	MPT_STRUCT(libhandle) lh = { 0, 0 };
	const char *err;
	
	/* bind from explicit/global search path */
	if ((err = mpt_library_assign(&lh, descr, path))) {
		if (out) mpt_log(out, __func__, path ? MPT_FCNLOG(Info) : MPT_FCNLOG(Error), "%s", err);
		return 0;
	}
	/* create remote instance */
	if (!(m = lh.create())) {
		mpt_library_close(&lh);
		if (out) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s", MPT_tr("error in library initializer"), descr);
		return 0;
	}
	if (!(mp = malloc(sizeof(*mp)))) {
		mpt_library_close(&lh);
		mpt_log(out, __func__, MPT_FCNLOG(Critical), "%s", MPT_tr("out of memory"));
		return 0;
	}
	mp->_mt._vptr = &_mpt_metaProxyCtl;
	mp->lh = lh;
	
	mp->px._ref = m;
	memset(&mp->px._types, 0, sizeof(mp->px._types));
	
	return &mp->_mt;
}
