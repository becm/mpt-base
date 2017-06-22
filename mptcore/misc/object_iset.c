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
	struct wrapIter *it = (void *) mt;
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeIterator), 's', 0 };
		if (dest) {
			*((const char **) dest) = fmt;
		}
		return *fmt;
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (dest) {
			*((const void **) dest) = it->it;
		}
		return 's';
	}
	return MPT_ERROR(BadType);
}

static int processIterator(void *ptr, MPT_INTERFACE(iterator) *it)
{
	static const MPT_INTERFACE_VPTR(metatype) ctl = {
		{ metaIterUnref },
		metaIterConv,
		metaIterClone
	};
	struct objectParam *par = ptr;
	MPT_INTERFACE(object) *obj = par->obj;
	struct wrapIter mt;
	
	mt._ctl._vptr = &ctl;
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
extern int mpt_object_iset(MPT_INTERFACE(object) *obj, const char *prop, MPT_STRUCT(value) *val)
{
	
	struct objectParam par;
	
	if (!(par.obj = obj)) {
		return MPT_ERROR(BadArgument);
	}
	par.prop = prop;
	
	return mpt_iterator_process(val, processIterator, &par);
}
