/*!
 * set metatype data.
 */

#include <stdlib.h>

#include "meta.h"

struct iteratorContent
{
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) *it;
};

static void iteratorUnref(MPT_INTERFACE(unrefable) *ref)
{
	struct iteratorContent *c = (void *) ref;
	MPT_INTERFACE(iterator) *it;
	
	if ((it = c->it)) {
		it->_vptr->ref.unref((void *) it);
	}
	free(c);
}
static int iteratorConv(const MPT_INTERFACE(metatype) *mt, int type, void *dest)
{
	struct iteratorContent *c = (void *) mt;
	
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeIterator) , 0 };
		if (dest) {
			*((const char **) dest) = fmt;
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (dest) {
			*((const void **) dest) = c->it;
		}
		return MPT_ENUM(TypeIterator);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *iteratorClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}

static const MPT_INTERFACE_VPTR(metatype) iteratorCtl = {
	{ iteratorUnref },
	iteratorConv,
	iteratorClone
};

/*!
 * \ingroup mptMeta
 * \brief set object property
 * 
 * Set property data to value.
 * Use iterator as intermediate for type conversions.
 * 
 * \param obj  object interface descriptor
 * \param prop name of property to change
 * \param val  data to set
 */
extern MPT_INTERFACE(metatype) *mpt_iterator_meta(MPT_INTERFACE(iterator) *it)
{
	struct iteratorContent *c;
	
	if (!(c = malloc(sizeof(*c)))) {
		return 0;
	}
	c->_mt._vptr = &iteratorCtl;
	c->it = it;
	
	return &c->_mt;
}
