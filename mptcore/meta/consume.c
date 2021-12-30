/*!
 * MPT conversion helper function
 *   unify value/iterator value conversion/advance
 */

#include <string.h>

#include "types.h"

#include "meta.h"

static int solverNext(MPT_STRUCT(consumable) *val, void *dest, int type, size_t len)
{
	MPT_INTERFACE(iterator) *it;
	int ret;
	if ((it = val->_it)) {
		union {
			double d;
			uintptr_t k;
			uint32_t u;
			int32_t i;
		} tmp;
		int next;
		if ((ret = it->_vptr->get(it, type, &tmp)) <= 0) {
			return ret;
		}
		if ((next = it->_vptr->advance(it)) < 0) {
			return next;
		}
		if (dest && len) {
			memcpy(dest, &tmp, len);
		}
		return ret;
	}
	if (val->_val.fmt) {
		uint8_t fmt;
		if (!(fmt = *val->_val.fmt)) {
			return 0;
		}
		if (fmt != type) {
			MPT_INTERFACE(convertable) *conv;
			if (fmt != MPT_ENUM(TypeConvertablePtr)) {
				return MPT_ERROR(BadType);
			}
			if (!(conv = *((MPT_INTERFACE(convertable) **) val->_val.ptr))) {
				return MPT_ERROR(BadValue);
			}
			if ((ret = conv->_vptr->convert(conv, type, dest)) < 0) {
				return ret;
			}
			len = sizeof(conv);
		}
		else if (dest) {
			memcpy(dest, val->_val.ptr, len);
		}
		val->_val.ptr = ((uint8_t *) val->_val.ptr) + len;
		return fmt;
	}
	if (type == 's'
	    || type == 'k') {
		if (!val->_val.ptr) {
			return 0;
		}
		if (dest) *((const char **) dest) = val->_val.ptr;
		val->_val.ptr = 0;
		return MPT_ENUM(TypeValue);
	}
	return MPT_ERROR(BadType);
}

/*!
 * \ingroup mptModule
 * \brief get float value
 * 
 * Convert module value element to key id.
 * 
 * \param val  module value data
 * \param ptr  target pointer
 * 
 * \return conversion result
 */
extern int mpt_consume_double(MPT_STRUCT(consumable) *val, double *ptr)
{
	return solverNext(val, ptr, 'd', sizeof(*ptr));
}
/*!
 * \ingroup mptModule
 * \brief get key value
 * 
 * Convert module value element to key id.
 * 
 * \param val  module value data
 * \param ptr  target pointer
 * 
 * \return conversion result
 */
extern int mpt_consume_key(MPT_STRUCT(consumable) *val, const char **ptr)
{
	return solverNext(val, ptr, 'k', sizeof(*ptr));
}
/*!
 * \ingroup mptModule
 * \brief get key value
 * 
 * Convert module value element to key id.
 * 
 * \param val  module value data
 * \param ptr  target pointer
 * 
 * \return conversion result
 */
extern int mpt_consume_uint(MPT_STRUCT(consumable) *val, uint32_t *ptr)
{
	return solverNext(val, ptr, 'u', sizeof(*ptr));
}
/*!
 * \ingroup mptModule
 * \brief get key value
 * 
 * Convert module value element to key id.
 * 
 * \param val  module value data
 * \param ptr  target pointer
 * 
 * \return conversion result
 */
extern int mpt_consume_int(MPT_STRUCT(consumable) *val, int32_t *ptr)
{
	return solverNext(val, ptr, 'i', sizeof(*ptr));
}
