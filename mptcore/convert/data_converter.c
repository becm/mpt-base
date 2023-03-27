/*!
 * MPT core library
 *   convert between MPT (base) type data
 */

#include <errno.h>

#include "types.h"
#include "meta.h"

#include "convert.h"

static int _mpt_convertable_wrap(const void *from, MPT_TYPE(value) type, void *dest)
{
	MPT_INTERFACE(convertable) *src;
	
	if (!from || !(src = *((void * const *) from))) {
		return MPT_ERROR(MissingData);
	}
	return src->_vptr->convert(src, type, dest);
}

static int _mpt_metatype_wrap(const void *from, MPT_TYPE(value) type, void *dest)
{
	MPT_INTERFACE(metatype) *mt;
	
	if (!from) {
		return MPT_ERROR(MissingData);
	}
	mt = *((MPT_INTERFACE(metatype) * const *) from);
	
	/* special processing of metatype references */
	if (type == MPT_ENUM(TypeMetaRef)) {
		MPT_INTERFACE(metatype) **ptr;
		/* only change reference for existing target */
		if ((ptr = dest)) {
			MPT_INTERFACE(metatype) *old;
			if (mt && !mt->_vptr->addref(mt)) {
				return MPT_ERROR(BadOperation);
			}
			if ((old = *ptr)) {
				old->_vptr->addref(old);
			}
			*ptr = mt;
		}
		return sizeof(*ptr);
	}
	if (!mt) {
		return MPT_ERROR(MissingData);
	}
	return MPT_metatype_convert(mt, type, dest);
}

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param type source data type
 * 
 * \return converte function for data type
 */

extern MPT_TYPE(data_converter) mpt_data_converter(MPT_TYPE(value) type)
{
	/* interfaces with builtin conversion */
	if (type == MPT_ENUM(TypeConvertablePtr)) {
		return _mpt_convertable_wrap;
	}
	if (type == MPT_ENUM(TypeMetaRef)) {
		return _mpt_metatype_wrap;
	}
	if (MPT_type_isMetaPtr(type)) {
		return _mpt_metatype_wrap;
	}
	if (type == MPT_ENUM(TypeArray)) {
		return (MPT_TYPE(data_converter)) mpt_data_convert_array;
	}
	if (type == MPT_ENUM(TypeBufferPtr)) {
		return (MPT_TYPE(data_converter)) mpt_data_convert_array;
	}
	switch (type) {
		/* signed types */
		case 'c': return (MPT_TYPE(data_converter)) mpt_data_convert_int8;
		case 'b': return (MPT_TYPE(data_converter)) mpt_data_convert_int8;
		case 'n': return (MPT_TYPE(data_converter)) mpt_data_convert_int16;
		case 'i': return (MPT_TYPE(data_converter)) mpt_data_convert_int32;
		case 'x': return (MPT_TYPE(data_converter)) mpt_data_convert_int64;
		/* unsigned types */
		case 'y': return (MPT_TYPE(data_converter)) mpt_data_convert_uint8;
		case 'q': return (MPT_TYPE(data_converter)) mpt_data_convert_uint16;
		case 'u': return (MPT_TYPE(data_converter)) mpt_data_convert_uint32;
		case 't': return (MPT_TYPE(data_converter)) mpt_data_convert_uint64;
		/* floating point */
		case 'f': return (MPT_TYPE(data_converter)) mpt_data_convert_float32;
		case 'd': return (MPT_TYPE(data_converter)) mpt_data_convert_float64;
#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e': return (MPT_TYPE(data_converter)) mpt_data_convert_exflt;
#endif
		default:;
	}
	
	errno = EINVAL;
	return 0;
}
