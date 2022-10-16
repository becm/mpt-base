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
 * \retval mpt::BadValue    conversion not allowed
 * \retval mpt::BadType     unknown conversion
 * \retval 0                no conversion performed
 * \retval 1                simple cast conversion
 * \retval 2                scalar to vector
 * \retval 3                use of special converter
 */

extern int mpt_value_convert(const MPT_STRUCT(value) *val, int type, void *dest)
{
	MPT_TYPE(data_converter) conv;
	const void *src = val->ptr;
	int ret;
	
	if (type <= 0 || !val->type) {
		return MPT_ERROR(BadArgument);
	}
	/* try specialized converter for type */
	if ((conv = mpt_data_converter(val->type))) {
		if ((ret = conv(src, type, dest)) >= 0) {
			return val->type == type ? 0 : 3;
		}
	}
	/* exact primitive type match */
	if (val->type == type) {
		const MPT_STRUCT(type_traits) *traits;
		if (!(traits = mpt_type_traits(type))) {
			return MPT_ERROR(BadArgument);
		}
		/* no raw copy for complex types */
		if (traits->init || traits->fini) {
			return MPT_ERROR(BadValue);
		}
		if (dest) {
			memcpy(dest, src, traits->size);
		}
		return 0;
	}
	/* vector source and genric target */
	if (type == MPT_ENUM(TypeVector)
	 && ((ret = MPT_type_toScalar(val->type)) > 0)) {
		const MPT_STRUCT(type_traits) *traits;
		if (!(traits = mpt_type_traits(ret))) {
			return MPT_ERROR(BadArgument);
		}
		/* content of generic vector must be primitive */
		if (traits->init || traits->fini) {
			return MPT_ERROR(BadValue);
		}
		if (dest) {
			memcpy(dest, src, sizeof(struct iovec));
		}
		return 1;
	}
	/* map to vector of same type with single element */
	if (type == MPT_type_toVector(val->type)) {
		const MPT_STRUCT(type_traits) *traits;
		struct iovec *vec;
		if (!(traits = mpt_type_traits(val->type))) {
			return MPT_ERROR(BadArgument);
		}
		/* convert from scalar */
		if ((vec = dest)) {
			vec->iov_base = (void *) src;
			vec->iov_len = traits->size;
		}
		return 2;
	}
	return MPT_ERROR(BadType);
}
