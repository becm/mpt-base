/*!
 * set metatype data.
 */

#include "meta.h"

#include "object.h"

struct objectParam
{
	MPT_INTERFACE(object) *obj;
	const char *prop;
};
struct wrapIter
{
	MPT_INTERFACE(metatype) _ctl;
	MPT_INTERFACE(iterator) *it;
};

static void metaIterUnref(MPT_INTERFACE(unrefable) *ref)
{
	(void) ref;
}
static MPT_INTERFACE(metatype) *metaIterClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static int metaIterConv(const MPT_INTERFACE(metatype) *mt, int type, void *dest)
{
	struct wrapIter *wr = (void *) mt;
	MPT_INTERFACE(iterator) *it;
	
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeIterator), MPT_ENUM(TypeValue), 0 };
		if (dest) {
			*((const char **) dest) = fmt;
		}
		return *fmt;
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (dest) {
			*((const void **) dest) = wr->it;
		}
		return MPT_ENUM(TypeValue);
	}
	if (!(it = wr->it)) {
		return MPT_ERROR(BadType);
	}
	return it->_vptr->get(it, type, dest);
}
static const MPT_INTERFACE_VPTR(metatype) metaIterCtl = {
	{ metaIterUnref },
	metaIterConv,
	metaIterClone
};

static int processIterator(void *ptr, MPT_INTERFACE(iterator) *it)
{
	struct objectParam *par = ptr;
	MPT_INTERFACE(object) *obj = par->obj;
	struct wrapIter mt;
	
	mt._ctl._vptr = &metaIterCtl;
	mt.it = it;
	
	return obj->_vptr->setProperty(obj, par->prop, &mt._ctl);
}
/*!
 * \ingroup mptObject
 * \brief set object property
 * 
 * Set property data to value.
 * Use iterator as intermediate for type conversions.
 * 
 * \param obj  object interface descriptor
 * \param prop name of property to change
 * \param val  data to set
 */
extern int mpt_object_nset(MPT_INTERFACE(object) *obj, const char *prop, MPT_STRUCT(value) *val)
{
	
	struct objectParam par;
	
	if (!(par.obj = obj)) {
		return MPT_ERROR(BadArgument);
	}
	par.prop = prop;
	
	return mpt_process_value(val, processIterator, &par);
}
/*!
 * \ingroup mptObject
 * \brief set object property
 * 
 * Set property data to value.
 * Use iterator as intermediate for type conversions.
 * 
 * \param obj  object interface descriptor
 * \param prop name of property to change
 * \param val  data to set
 */
extern int mpt_object_iset(MPT_INTERFACE(object) *obj, const char *prop, MPT_INTERFACE(iterator) *it)
{
	struct wrapIter mt;
	
	mt._ctl._vptr = &metaIterCtl;
	mt.it = it;
	
	return obj->_vptr->setProperty(obj, prop, &mt._ctl);
}
