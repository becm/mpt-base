/*!
 * set object data from iterator.
 */

#include "object.h"
#include "convert.h"
#include "types.h"

#include "meta.h"

struct valueConverter
{
	MPT_INTERFACE(convertable) _ctl;
	MPT_TYPE(data_converter) conv;
	const void *data;
};

static int valueConvert(MPT_INTERFACE(convertable) *conv, int type, void *ptr)
{
	const struct valueConverter *val = (void *) conv;
	return val->conv(val->data, type, ptr);
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
extern int mpt_object_set_value(MPT_INTERFACE(object) *obj, const char *prop, const MPT_STRUCT(value) *val)
{
	static const MPT_INTERFACE_VPTR(convertable) ctl = { valueConvert };
	struct valueConverter conv = { { &ctl }, 0, 0 };
	
	if (!val) {
		return obj->_vptr->set_property(obj, prop, 0);
	}
	if (!(conv.conv = mpt_data_converter(val->type))) {
		return MPT_ERROR(BadType);
	}
	conv.data = val->ptr;
	return obj->_vptr->set_property(obj, prop, &conv._ctl);
}
