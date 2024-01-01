/*!
 * set object data from iterator.
 */

#include "types.h"

#include "object.h"

struct convIter
{
	MPT_INTERFACE(convertable) _ctl;
	MPT_INTERFACE(iterator) *it;
};

static int iteratorConv(MPT_INTERFACE(convertable) *mt, MPT_TYPE(type) type, void *dest)
{
	const struct convIter *wr = (void *) mt;
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), 0 };
		if (dest) {
			*((const uint8_t **) dest) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (dest) *((const void **) dest) = wr->it;
		return MPT_ENUM(TypeIteratorPtr);
	}
	return MPT_ERROR(BadType);
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
extern int mpt_object_set_iterator(MPT_INTERFACE(object) *obj, const char *prop, MPT_INTERFACE(iterator) *it)
{
	static const MPT_INTERFACE_VPTR(convertable) convIterCtl = {
		iteratorConv
	};
	struct convIter conv;
	
	conv._ctl._vptr = &convIterCtl;
	conv.it = it;
	
	return obj->_vptr->set_property(obj, prop, &conv._ctl);
}
