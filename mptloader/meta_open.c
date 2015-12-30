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
};

static int mpUnref(MPT_INTERFACE(metatype) *m)
{
	struct _mpt_metaProxy *mp = (void *) m;
	mp->_d->_vptr->unref(mp->_d);
	mpt_library_close(&mp->lh);
	free(mp);
	return 0;
}
static MPT_INTERFACE(metatype) *mpAddref(MPT_INTERFACE(metatype) *m)
{
	(void) m;
	return 0;
}
static int mpAssign(MPT_INTERFACE(metatype) *m, const MPT_STRUCT(value) *val)
{
	struct _mpt_metaProxy *mp = (void *) m;
	return mp->_d->_vptr->assign(mp->_d, val);
}
static void *mpTypecast(MPT_INTERFACE(metatype) *m, int type)
{
	struct _mpt_metaProxy *mp = (void *) m;
	return mp->_d->_vptr->typecast(mp->_d, type);
}

static const MPT_INTERFACE_VPTR(metatype) _mpt_metaProxyCtl = {
	mpUnref,
	mpAddref,
	mpAssign,
	mpTypecast
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
	if ((obj = m->_vptr->typecast(m, MPT_ENUM(TypeObject)))) {
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
	mp->_mt._vptr = &_mpt_metaProxyCtl;
	
	return &mp->_mt;
}
