/*!
 * MPT consumable setup
 *   set consumable data from matching metatype conversion
 */

#include "meta.h"

/*!
 * \ingroup mptModule
 * \brief set consumable
 * 
 * Convert metatype to consumable data type.
 * 
 * \param val  consumable data
 * \param mt   source metatype
 * 
 * \return conversion result
 */
extern int mpt_consumable_setup(MPT_STRUCT(consumable) *val, const MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(value) tmp;
	int ret;
	
	if ((ret = mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeIterator)), &val->_it)) >= 0) {
		val->_val.fmt = 0;
		val->_val.ptr = 0;
		if (!val->_it) {
			return 0;
		}
		return MPT_ENUM(TypeIterator);
	}
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeValue), &tmp)) >= 0) {
		if (tmp.fmt && !tmp.ptr) {
			return MPT_ERROR(BadValue);
		}
		val->_it = 0;
		val->_val = tmp;
		return MPT_ENUM(TypeValue);
	}
	if ((ret = mt->_vptr->conv(mt, 's', &val->_val.ptr)) >= 0) {
		val->_it = 0;
		val->_val.fmt = 0;
		return 's';
	}
	return MPT_ERROR(BadType);
}
