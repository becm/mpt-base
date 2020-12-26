/*!
 * MPT consumable setup
 *   set consumable data from matching metatype conversion
 */

#include "types.h"

#include "meta.h"

/*!
 * \ingroup mptModule
 * \brief set consumable
 * 
 * Convert metatype to consumable data type.
 * 
 * \param val  consumable data
 * \param src  source convertable
 * 
 * \return conversion result
 */
extern int mpt_consumable_setup(MPT_STRUCT(consumable) *val, MPT_INTERFACE(convertable) *src)
{
	MPT_STRUCT(value) tmp;
	int ret;
	
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &val->_it)) >= 0) {
		val->_val.fmt = 0;
		val->_val.ptr = 0;
		if (!val->_it) {
			return 0;
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeValue), &tmp)) >= 0) {
		if (tmp.fmt && !tmp.ptr) {
			return MPT_ERROR(BadValue);
		}
		val->_it = 0;
		val->_val = tmp;
		return MPT_ENUM(TypeValue);
	}
	if ((ret = src->_vptr->convert(src, 's', &val->_val.ptr)) >= 0) {
		val->_it = 0;
		val->_val.fmt = 0;
		return 's';
	}
	return MPT_ERROR(BadType);
}
