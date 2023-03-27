/*!
 * set object data from iterator.
 */

#include <string.h>

#include "convert.h"
#include "types.h"

#include "object.h"

static int valueConvert(MPT_INTERFACE(convertable) *conv, MPT_TYPE(value) type, void *ptr)
{
	const MPT_STRUCT(value) *val = *((void **) (conv + 1));
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeValue), 0 };
		if (ptr) *((const uint8_t **) ptr) = fmt;
		return val->_type;
	}
	/* verbatim value assignment */
	if (type == MPT_ENUM(TypeValue)) {
		if (ptr) {
			memcpy(ptr, val, sizeof(*val));
		}
		return type;
	}
	return mpt_value_convert(val, type, ptr);
}

/*!
 * \ingroup mptObject
 * \brief set object property
 * 
 * Set property data to value.
 * Allow access to verbatim value
 * or try to convert/copy value to target type.
 * 
 * \param obj  object interface descriptor
 * \param prop name of property to change
 * \param val  data to set
 */
extern int mpt_object_set_value(MPT_INTERFACE(object) *obj, const char *prop, const MPT_STRUCT(value) *val)
{
	static const MPT_INTERFACE_VPTR(convertable) ctl = { valueConvert };
	struct {
		MPT_INTERFACE(convertable) _ctl;
		const MPT_STRUCT(value) *val;
	} conv = { { &ctl }, 0 };
	
	if (!(conv.val = val)) {
		return obj->_vptr->set_property(obj, prop, 0);
	}
	return obj->_vptr->set_property(obj, prop, &conv._ctl);
}
