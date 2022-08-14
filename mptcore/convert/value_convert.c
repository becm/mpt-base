/*!
 * MPT core library
 *   convert between MPT (base) type data
 */

#include <string.h>

#include <sys/uio.h>

#include "types.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * Apply known/allowed conversion to target data.
 * 
 * \param val   source data
 * \param type  target data type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval 0                no conversion performed
 * \retval 1                simple cast conversion
 * \retval 2                scalar to vector
 * \retval 3                use of special converter
 * \retval 4                string conversion
 */

extern int mpt_value_convert(const MPT_STRUCT(value) *val, int type, void *dest)
{
	const MPT_STRUCT(type_traits) *traits;
	MPT_TYPE(data_converter) conv;
	const void *src = val->ptr;
	const char *str;
	int ret;
	
	/* try specialized converter for type */
	if ((conv = mpt_data_converter(val->type))) {
		if ((ret = conv(src, type, dest)) >= 0) {
			return val->type == type ? 0 : 3;
		}
	}
	if ((traits = mpt_type_traits(val->type))) {
		/* exact primitive type match */
		if (val->type == type && !traits->init && !traits->fini) {
			size_t len = traits->size;
			if (dest) {
				memcpy(dest, src, len);
			}
			return 0;
		}
	}
	/* allow broad primitive data conversion */
	if (traits && MPT_type_isVector(type) && !traits->fini && !traits->init) {
		struct iovec *vec = dest;
		size_t len = traits->size;
		
		/* allow generic more target type */
		if (MPT_type_isVector(val->type) && type == MPT_ENUM(TypeVector)) {
			if (vec) {
				memcpy(vec, src, len);
			}
			return 1;
		}
		/* convert from scalar */
		else if ((type == MPT_ENUM(TypeVector)) || (type == MPT_type_toVector(val->type))) {
			if (vec) {
				vec->iov_base = (void *) src;
				vec->iov_len = len;
			}
			return 2;
		}
	}
	if ((str = mpt_data_tostring(&src, val->type, 0))) {
		if ((ret = mpt_convert_string(str, type, dest)) >= 0) {
			return 4;
		}
	}
	return MPT_ERROR(BadType);
}
