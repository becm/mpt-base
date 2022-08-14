
#include <ctype.h>
#include <string.h>

#include "convert.h"

#include "types.h"

/*!
 * \ingroup mptMeta
 * \brief consume iterator data
 * 
 * Get data from iterator, convert type and advance iterator.
 * Only works for fully self-suffient target types (number values).
 * 
 * \param it    interface to consume value from
 * \param type  convertion target type
 * \param dest  target data address
 * 
 * \return length of converted type
 */
extern int mpt_iterator_consume(MPT_INTERFACE(iterator) *it, int type, void *dest)
{
	const MPT_STRUCT(value) *val = it->_vptr->value(it);
	uint8_t tmp[32];
	size_t len;
	int ret;
	
	/* skip current value */
	if (!type) {
		if (val && !val->_namespace) {
			type = val->type;
		}
		if ((ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
		return type;
	}
	/* no data to work with */
	if (!val) {
		return MPT_ERROR(MissingData);
	}
	/* limit options to known self-sufficent types */
	switch (type) {
		case 'c': len = sizeof(char); break;
		case 'b': len = sizeof(int8_t); break;
		case 'y': len = sizeof(uint8_t); break;
		case 'n': len = sizeof(int16_t); break;
		case 'q': len = sizeof(uint16_t); break;
		case 'i': len = sizeof(int32_t); break;
		case 'u': len = sizeof(uint32_t); break;
		case 'x': len = sizeof(int64_t); break;
		case 't': len = sizeof(uint64_t); break;
		
		case 'l': len = sizeof(long); break;
		
		case 'f': len = sizeof(float); break;
		case 'd': len = sizeof(double); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e': len = sizeof(long double); break;
#endif
		default: return MPT_ERROR(BadType);
	}
	if ((ret = mpt_value_convert(val, type, dest ? tmp : 0)) < 0) {
		return ret;
	}
	if ((ret = it->_vptr->advance(it) < 0)) {
		return ret;
	}
	if (dest) {
		memcpy(dest, tmp, len);
	}
	return val->type;
}
