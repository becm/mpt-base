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
	MPT_STRUCT(proxy)        px;
	MPT_STRUCT(libhandle)    lh;
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
	if (strchr(mp->px._types, MPT_ENUM(TypeMeta))) {
		return m->_vptr->assign(m, val);
	}
	if (!strchr(mp->px._types, MPT_ENUM(TypeObject))) {
		return MPT_ERROR(BadOperation);
	}
	obj = mp->px._ref;
	if (!val) {
		return obj->_vptr->setProperty(obj, 0, 0);
	}
	return mpt_object_set(mp->px._ref, 0, fmt, *val);
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
 * \param out   logging instance
 * 
 * \return metatype proxy pointer
 */
extern MPT_INTERFACE(metatype) *mpt_meta_open(const char *descr, const char *path, MPT_INTERFACE(logger) *out)
{
	struct _mpt_metaProxy *mp;
	MPT_INTERFACE(metatype) *m;
	MPT_INTERFACE(object) *obj;
	MPT_STRUCT(proxy) px;
	MPT_STRUCT(libhandle) lh = { 0, 0 };
	const char *err;
	int len;
	
	/* get type values */
	memset(&px._types, 0, sizeof(px._types));
	if ((len = mpt_proxy_type(&px, descr)) < 0) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s", MPT_tr("bad reference type name"), descr);
		return 0;
	}
	/* bind from explicit/global search path */
	if (!(err = mpt_library_assign(&lh, descr+len, path))) {
		(void) mpt_log(out, __func__, path ? MPT_FCNLOG(Info) : MPT_FCNLOG(Error), "%s", err);
		return 0;
	}
	/* create remote instance */
	if (!(m = lh.create())) {
		mpt_library_close(&lh);
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s", MPT_tr("error in library initializer"), descr+len);
		return 0;
	}
	if (strchr(px._types, MPT_ENUM(TypeMeta))
	    && m->_vptr->conv(m, MPT_ENUM(TypeObject), &obj) >= 0 && obj) {
		err = mpt_object_typename(obj);
		if (!err) err = "";
		(void) mpt_log(out, __func__, MPT_FCNLOG(Debug), "%s: %s", MPT_tr("created proxy instance"), err);
	}
	if (!(mp = malloc(sizeof(*mp)))) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Critical), "%s", MPT_tr("out of memory"));
		return 0;
	}
	mp->lh = lh;
	mp->px = px;
	mp->_mt._vptr = &_mpt_metaProxyCtl;
	
	return &mp->_mt;
}
