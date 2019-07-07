/*!
 * initialize solver from shared library.
 */

#include <stdlib.h>
#include <string.h>

#include "meta.h"
#include "object.h"
#include "output.h"

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

/* convertable interface */
static int _proxy_conv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	const struct _mpt_metaProxy *mp = (void *) val;
	int mt = mp->sym.type;
	
	if (!(val = mp->ptr)) {
		return MPT_ERROR(BadArgument);
	}
	if (!type) {
		if (ptr) *((void **) ptr) = (char *) mp->fmt;
		return mt;
	}
	if (type == MPT_type_pointer(mt)) {
		if (ptr) *((void **) ptr) = mp->ptr;
		return mt;
	}
	if (type == 's') {
		if (ptr) *((const void **) ptr) = mp + 1;
		return mt;
	}
	if (!MPT_type_isMetatype(mt)) {
		return MPT_ERROR(BadType);
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (ptr) *((void **) ptr) = mp->ptr;
		return mt;
	}
	return val->_vptr->convert(val, type, ptr);
}
/* metatype interface */
static void _proxy_unref(MPT_INTERFACE(metatype) *mt)
{
	struct _mpt_metaProxy *mp = (void *) mt;
	
	if (__mpt_library_proxy_unref(&mp->_ref)) {
		return;
	}
	if ((mt = mp->ptr)) {
		int type = mp->sym.type;
		if (MPT_type_isMetatype(type)) {
			mt->_vptr->unref(mt);
		}
	}
	mpt_library_detach(&mp->sym.lib);
	free(mp);
}
static uintptr_t _proxy_addref(MPT_INTERFACE(metatype) *mt)
{
	struct _mpt_metaProxy *mp = (void *) mt;
	return __mpt_library_proxy_addref(&mp->_ref);
}
static MPT_INTERFACE(metatype) *_proxy_clone(const MPT_INTERFACE(metatype) *m)
{
	const struct _mpt_metaProxy *mp = (void *) m;
	MPT_STRUCT(libhandle) *lh = 0;
	struct _mpt_metaProxy *n;
	union {
		void *ptr;
		void *(*make)(void);
	} val;
	size_t size;
	
	if (!(val.ptr = mp->sym.addr)) {
		return 0;
	}
	if (mp->sym.lib && !(lh = mpt_library_attach(mp->sym.lib))) {
		return 0;
	}
	size = sizeof(*n) + mp->len + 1;
	if (!(n = malloc(size))) {
		return 0;
	}
	memcpy(n, mp, size);
	n->_ref._val = 1;
	n->sym.lib = lh;
	
	if (!(n->ptr = val.make())) {
		_proxy_unref(&n->_mt);
		return 0;
	}
	return &n->_mt;
}

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
	static const MPT_INTERFACE_VPTR(metatype) _proxy_ctl = {
		{ _proxy_conv },
		_proxy_unref,
		_proxy_addref,
		_proxy_clone
	};
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
	
	mp->_mt._vptr = &_proxy_ctl;
	mp->_ref._val = 1;
	
	mp->sym = sym;
	mp->ptr = val.ptr;
	mp->len = len;
	
	if (type >= 0 && type <= 0xff) {
		mp->fmt[0] = type;
	} else {
		mp->fmt[0] = 0;
	}
	mp->fmt[1] = 0;
	
	desc = memcpy(mp + 1, desc, len + 1);
	
	return &mp->_mt;
}
