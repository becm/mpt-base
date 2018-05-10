/*!
 * initialize solver from shared library.
 */

#include <stdlib.h>
#include <string.h>

#include "meta.h"
#include "object.h"

#include "loader.h"

extern uintptr_t __mpt_library_proxy_unref(MPT_STRUCT(refcount) *);
extern uintptr_t __mpt_library_proxy_addref(MPT_STRUCT(refcount) *);

struct _mpt_metaProxy
{
	MPT_INTERFACE(metatype) _mt;
	MPT_STRUCT(refcount) _ref;
	MPT_STRUCT(libsymbol) sym;
	void *ptr;
	uint16_t len;
	uint8_t fmt[2];
};

static void mpUnref(MPT_INTERFACE(reference) *m)
{
	struct _mpt_metaProxy *mp = (void *) m;
	
	if (__mpt_library_proxy_unref(&mp->_ref)) {
		return;
	}
	if ((m = mp->ptr)) {
		int type = mp->fmt[0];
		if (MPT_value_isMetatype(type)) {
			m->_vptr->unref(m);
		}
	}
	mpt_library_detach(&mp->sym.lib);
	free(mp);
}
static uintptr_t mpRef(MPT_INTERFACE(reference) *ref)
{
	struct _mpt_metaProxy *mp = (void *) ref;
	return __mpt_library_proxy_addref(&mp->_ref);
}
static int mpConv(const MPT_INTERFACE(metatype) *m, int type, void *ptr)
{
	const struct _mpt_metaProxy *mp = (void *) m;
	int mt = mp->sym.type;
	
	if (!(m = mp->ptr)) {
		return MPT_ERROR(BadArgument);
	}
	if (!type) {
		if (ptr) *((void **) ptr) = (char *) mp->fmt;
		return mt;
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
static const MPT_INTERFACE_VPTR(metatype) _mpt_metaProxyCtl;
static MPT_INTERFACE(metatype) *mpClone(const MPT_INTERFACE(metatype) *m)
{
	const struct _mpt_metaProxy *mp = (void *) m;
	struct _mpt_metaProxy *n;
	union {
		void *ptr;
		void *(*make)(void);
	} val;
	size_t size;
	
	if (!(val.ptr = mp->sym.addr)) {
		return 0;
	}
	size = sizeof(*n) + mp->len + 1;
	if (!(n = malloc(size))) {
		return 0;
	}
	if (mp->sym.lib && !__mpt_library_proxy_addref(&mp->sym.lib->_ref)) {
		free(n);
		return 0;
	}
	memcpy(n, mp, size);
	n->_ref._val = 1;
	
	if (!(n->ptr = val.make())) {
		mpUnref((MPT_INTERFACE(reference) *) n);
		return 0;
	}
	return &n->_mt;
}
static const MPT_INTERFACE_VPTR(metatype) _mpt_metaProxyCtl = {
	{ mpUnref, mpRef },
	mpConv,
	mpClone
};

static void msg(MPT_INTERFACE(logger) *info, const char *fcn, int type, const char *fmt, ... )
{
	va_list va;
	if (!info) {
		return;
	}
	va_start(va, fmt);
	info->_vptr->log(info, fcn, type, fmt, va);
	va_end(va);
}

/*!
 * \ingroup mptLoader
 * \brief metatype proxy
 * 
 * Create metatype proxy to library instance.
 * 
 * \param type  type of instance to create
 * \param desc  library symbol and name
 * \param path  alternate path for library
 * \param info  log/error output instance
 * 
 * \return metatype proxy pointer
 */
extern MPT_INTERFACE(metatype) *mpt_library_meta(int type, const char *desc, const char *path, MPT_INTERFACE(logger) *info)
{
	MPT_STRUCT(libsymbol) sym = MPT_LIBSYMBOL_INIT;
	struct _mpt_metaProxy *mp;
	union {
		void *ptr;
		void *(*make)(void);
	} val;
	size_t len;
	int ret;
	
	if (!desc) {
		msg(info, __func__, MPT_LOG(Error), "%s: %s",
		    MPT_tr("no proxy description"));
		return 0;
	}
	if (type < 0) {
		msg(info, __func__, MPT_LOG(Debug2), "%s: %s",
		    MPT_tr("unknown instance type"), desc);
		return 0;
	}
	if ((len = strlen(desc)) > UINT16_MAX) {
		ret = len;
		msg(info, __func__, MPT_LOG(Error), "%s (%d)",
		    MPT_tr("symbol name too big"), ret);
		return 0;
	}
	if ((ret = mpt_library_bind(&sym, desc, path, info)) < 0) {
		return 0;
	}
	if (!(val.ptr = sym.addr) || !(val.ptr = val.make())) {
		msg(info, __func__, MPT_LOG(Error), "%s: %s",
		    MPT_tr("unable to create instance"), desc);
		mpt_library_detach(&sym.lib);
		return 0;
	}
	if (!(mp = malloc(sizeof(*mp) + len + 1))) {
		mpt_library_detach(&sym.lib);
		return 0;
	}
	sym.type = type;
	
	mp->_mt._vptr = &_mpt_metaProxyCtl;
	mp->_ref._val = 1;
	
	mp->sym = sym;
	mp->ptr = val.ptr;
	mp->len = len;
	
	if (MPT_value_isMetatype(type)) {
		mp->fmt[0] = MPT_ENUM(TypeMeta);
	}
	else if (type >= 0 && type <= 0xff) {
		mp->fmt[0] = type;
	}
	else {
		mp->fmt[0] = 0;
	}
	mp->fmt[1] = 0;
	
	desc = memcpy(mp + 1, desc, len + 1);
	
	return &mp->_mt;
}
