/*!
 * MPT core library
 *   convert between MPT (base) type data
 */

#include <ctype.h>
#include <string.h>
#include <float.h>

#include <sys/uio.h>

#include "array.h"
#include "object.h"
#include "types.h"

#include "meta.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed convertion between different data types.
 * 
 * \param fptr  source data
 * \param ftype source data type
 * \param dest  destination pointer
 * \param dtype destination data type
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destiantion data size
 */

extern int mpt_data_convert(const void **fptr, int ftype, void *dest, int dtype)
{
	const MPT_STRUCT(type_traits) *traits;
	MPT_TYPE(data_converter) conv;
	const uint8_t *from = *fptr;
	
	if (!(traits = mpt_type_traits(ftype))) {
		return MPT_ERROR(BadType);
	}
	/* exact primitive type match */
	if (ftype == dtype && !traits->init && !traits->fini) {
		size_t len = traits->size;
		if (dest) {
			memcpy(dest, from, len);
		}
		*fptr = from + len;
		return len;
	}
	/* try specialized converter for type */
	if ((conv = mpt_data_converter(ftype))) {
		int ret;
		if ((ret = conv(from, dtype, dest)) < 0) {
			return ret;
		}
		*fptr = from + traits->size;
		return ret;
	}
	/* allow broad primitive data conversion */
	if (MPT_type_isVector(dtype) && !traits->fini && !traits->init) {
		struct iovec *vec = dest;
		size_t len = traits->size;
		
		/* require generic target type */
		if (MPT_type_isVector(ftype) && dtype == MPT_ENUM(TypeVector)) {
			if (vec) {
				memcpy(vec, from, len);
			}
		}
		/* convert from scalar */
		else if ((dtype == MPT_ENUM(TypeVector)) || (dtype == MPT_type_toVector(ftype))) {
			if (vec) {
				vec->iov_base = (void *) from;
				vec->iov_len = len;
			}
		}
		else {
			return MPT_ERROR(BadType);
		}
		*fptr = from + len;
		return sizeof(*vec);
	}
	return MPT_ERROR(BadType);
}
