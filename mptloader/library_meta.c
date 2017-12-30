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
	void *ptr;
	MPT_STRUCT(libhandle) lh;
	uint16_t len;
	char fmt[2];
};

static void mpUnref(MPT_INTERFACE(reference) *m)
{
	struct _mpt_metaProxy *mp = (void *) m;
	
	if ((m = mp->ptr)) {
		if (mp->lh.type) {
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
	int mt = mp->lh.type;
	
	if (!(m = mp->ptr)) {
		return MPT_ERROR(BadArgument);
	}
	if (!type) {
		if (ptr) *((void **) ptr) = (char *) mp->fmt;
		return mp->lh.type;
	}
	if (type == mt) {
		if (ptr) *((void **) ptr) = mp->ptr;
		return mp->fmt[0];
	}
	if (type == 's') {
		if (ptr) *((const void **) ptr) = mp + 1;
		return mp->fmt[0];
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (!MPT_value_isMetatype(mt)) {
			return MPT_ERROR(BadType);
		}
		if (ptr) *((void **) ptr) = mp->ptr;
		return mt;
	}
	if (MPT_value_isMetatype(mt)) {
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
	if (!(n = malloc(sizeof(*n) + mp->len + 1))) {
		return 0;
	}
	*n = *mp;
	n->ptr = ptr;
	n->fmt[0] = mp->fmt[0];
	n->fmt[1] = 0;
	
	memcpy(n + 1, mp + 1, mp->len + 1);
	
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
 * \param desc  library symbol and name
 * \param path  alternate path for library
 * \param type  type of instance to create
 * 
 * \return metatype proxy pointer
 */
extern MPT_INTERFACE(metatype) *mpt_library_meta(const char *desc, const char *path, MPT_INTERFACE(logger) *info)
{
	MPT_STRUCT(libhandle) lh = MPT_LIBHANDLE_INIT;
	struct _mpt_metaProxy *mp;
	MPT_INTERFACE(metatype) *mt;
	const char *sym;
	size_t len;
	int ret;
	
	if (!desc) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %s",
			        MPT_tr("no proxy description"));
		}
		return 0;
	}
	if (!(sym = strchr(desc, ':'))) {
		ret = MPT_ENUM(TypeMeta);
		sym = desc;
	}
	else if ((ret = mpt_proxy_typeid(desc, &sym)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %s",
			        MPT_tr("bad type description"), desc);
		}
		return 0;
	}
	if ((len = strlen(sym)) > UINT16_MAX) {
		if (info) {
			ret = len;
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%d)",
			        MPT_tr("symbol nam too big"), len);
		}
		return 0;
	}
	lh.type = ret;
	if ((ret = mpt_library_bind(&lh, sym, path, info)) < 0) {
		return 0;
	}
	if (!(mt = lh.create())) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %s",
			        MPT_tr("unable to create instance"), desc);
		}
		mpt_library_close(&lh);
	}
	if (!(mp = malloc(sizeof(*mp) + len + 1))) {
		return 0;
	}
	mp->_mt._vptr = &_mpt_metaProxyCtl;
	mp->lh = lh;
	mp->ptr = mt;
	
	mp->len = len;
	mp->fmt[0] = (lh.type >= 0 && lh.type <= 0xff) ? lh.type : 0;
	mp->fmt[1] = 0;
	
	sym = memcpy(mp + 1, sym, len + 1);
	
	if (info) {
		MPT_INTERFACE(object) *obj = 0;
		if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0
		    && obj) {
			if (!(sym = mpt_object_typename(obj))) {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s",
				        MPT_tr("created proxy object"));
			} else {
				mpt_log(info, __func__, MPT_LOG(Debug), "%s: %s",
				        MPT_tr("created proxy object"), sym);
			}
		}
		/* named instance */
		else if ((sym = mpt_valtype_name(lh.type))) {
			mpt_log(info, __func__, MPT_LOG(Debug), "%s: %s",
			        MPT_tr("created proxy instance"), sym);
		}
		/* generic instance */
		else {
			ret = lh.type;
			mpt_log(info, __func__, MPT_LOG(Debug), "%s: %02x",
			        MPT_tr("created proxy instance"), ret);
		}
	}
	return &mp->_mt;
}
