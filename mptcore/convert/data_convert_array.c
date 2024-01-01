/*!
 * MPT core library
 *   convert between MPT (base) type data
 */

#include <string.h>

#include <sys/uio.h>

#include "array.h"
#include "types.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief convert data 
 * 
 * apply known/allowed assignments for array data or content.
 * 
 * \param from  source data
 * \param type  target type
 * \param dest  destination pointer
 * 
 * \retval mpt::BadArgument unknown source/target type
 * \retval mpt::BadValue    conversion not in allowd range
 * \retval mpt::BadType     unknown conversion
 * \retval >0               destination data size
 */
extern int mpt_data_convert_array(const MPT_STRUCT(array) *from, MPT_TYPE(type) type, void *dest)
{
	const MPT_STRUCT(array) *arr = (void *) from;
	const MPT_STRUCT(type_traits) *traits;
	MPT_STRUCT(buffer) *b = arr->_buf;
	int scalar;
	
	/* special content on buffer */
	if (type == MPT_ENUM(TypeBufferPtr)) {
		if (dest) {
			*((MPT_STRUCT(buffer) **) dest) = b;
			return MPT_ERROR(BadOperation);
		}
		return sizeof(*arr);
	}
	
	if (type == 's') {
		const char *s;
		if (!b || !(traits = b->_content_traits)) {
			return MPT_ERROR(MissingData);
		}
		if (traits != mpt_type_traits('c')) {
			return MPT_ERROR(BadType);
		}
		if (!(s = memchr(b + 1, 0, b->_used))) {
			return MPT_ERROR(BadValue);
		}
		if (dest) {
			const char **txt = dest;
			*txt = (void *) (b + 1);
		}
		return sizeof(*arr);
	}
	if ((scalar = MPT_type_toScalar(type)) >= 0) {
		struct iovec *vec;
		traits = b ? b->_content_traits : 0;
		
		if (!traits) {
			if (type != MPT_ENUM(TypeVector)) {
				return MPT_ERROR(BadType);
			}
		}
		else if (traits != mpt_type_traits(scalar)) {
			return MPT_ERROR(BadType);
		}
		if ((vec = dest)) {
			if (b) {
				vec->iov_base = b + 1;
				vec->iov_len = b->_used;
			} else {
				vec->iov_base = 0;
				vec->iov_len = 0;
			}
		}
		return sizeof(*arr);
	}
	
	return MPT_ERROR(BadType);
}
