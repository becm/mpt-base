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
	size_t size;
	int type;
};

static int valueConvert(MPT_INTERFACE(convertable) *conv, int type, void *ptr)
{
	const struct valueConverter *val = (void *) conv;
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeValue), 0 };
		if (ptr) *((const uint8_t **) ptr) = fmt;
		return val->type;
	}
	if (val->conv) {
		int ret = val->conv(val->data, type, ptr);
		if (ret >= 0) {
			return ret;
		}
	}
	if (type == val->type && val->size) {
		if (ptr) memcpy(ptr, val->data, val->size);
		return MPT_ENUM(TypeValue);
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
extern int mpt_object_set_value(MPT_INTERFACE(object) *obj, const char *prop, const MPT_STRUCT(value) *val)
{
	static const MPT_INTERFACE_VPTR(convertable) ctl = { valueConvert };
	const MPT_STRUCT(type_traits) *traits;
	struct valueConverter conv = { { &ctl }, 0, 0, 0, 0 };
	
	if (!val) {
		return obj->_vptr->set_property(obj, prop, 0);
	}
	conv.conv = mpt_data_converter(val->type);
	traits = mpt_type_traits(val->type);
	if (traits && !traits->init && !traits->fini && traits->size) {
		conv.size = traits->size;
	}
	else if (!conv.conv) {
		return MPT_ERROR(BadType);
	}
	conv.type = val->type;
	conv.data = val->ptr;
	return obj->_vptr->set_property(obj, prop, &conv._ctl);
}
