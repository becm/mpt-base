/*!
 * initialize solver from shared library.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

struct _mpt_metaProxy
{
	MPT_INTERFACE(metatype) _mt, *_d;
	MPT_STRUCT(libhandle)    lh;
	MPT_STRUCT(reference)   _ref;
};

static void mpUnref(MPT_INTERFACE(metatype) *m)
{
	struct _mpt_metaProxy *mp = (void *) m;
	uintptr_t c;
	mp->_d->_vptr->unref(mp->_d);
	if ((c = mpt_reference_lower(&mp->_ref))) {
		return;
	}
	mpt_library_close(&mp->lh);
	free(mp);
}
static int mpAssign(MPT_INTERFACE(metatype) *m, const MPT_STRUCT(value) *val)
{
	struct _mpt_metaProxy *mp = (void *) m;
	return mp->_d->_vptr->assign(mp->_d, val);
}
static int mpConv(MPT_INTERFACE(metatype) *m, int type, void *ptr)
{
	struct _mpt_metaProxy *mp = (void *) m;
	return mp->_d->_vptr->conv(mp->_d, type, ptr);
}
static MPT_INTERFACE(metatype) *mpClone(MPT_INTERFACE(metatype) *m)
{
	struct _mpt_metaProxy *mp = (void *) m;
	MPT_INTERFACE(metatype) *ref;
	
	if (!(ref = mp->_d->_vptr->clone(mp->_d))) {
		return 0;
	}
	/* require multi reference to keep count */
	if (ref != mp->_d || !mpt_reference_raise(&mp->_ref)) {
		ref->_vptr->unref(ref);
		return 0;
	}
	return m;
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
extern MPT_INTERFACE(metatype) *mpt_meta_open(const char *descr, MPT_INTERFACE(logger) *out)
{
	struct _mpt_metaProxy *mp;
	MPT_INTERFACE(metatype) *m;
	MPT_INTERFACE(object) *obj;
	MPT_STRUCT(libhandle) lh = { 0, 0 };
	const char *err;
	
	/* bind library handle */
	if ((err = mpt_library_assign(&lh, descr))) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s", err);
		return 0;
	}
	/* create remote instance */
	if (!(m = lh.create())) {
		mpt_library_close(&lh);
		(void) mpt_log(out, __func__, MPT_FCNLOG(Error), "%s: %s", MPT_tr("error in library initializer"), descr);
		return 0;
	}
	if (m->_vptr->conv(m, MPT_ENUM(TypeObject), &obj) >= 0 && obj) {
		err = mpt_object_typename(obj);
		if (!err) err = "";
		(void) mpt_log(out, __func__, MPT_FCNLOG(Debug), "%s: %s", MPT_tr("created proxy instance"), err);
	}
	if (!(mp = malloc(sizeof(*mp)))) {
		(void) mpt_log(out, __func__, MPT_FCNLOG(Critical), "%s", MPT_tr("out of memory"));
		return 0;
	}
	mp->lh = lh;
	mp->_d = m;
	mp->_ref._val = 1;
	mp->_mt._vptr = &_mpt_metaProxyCtl;
	
	return &mp->_mt;
}
