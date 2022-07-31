/*!
 * configure and prepare bound solver.
 */

#include <ctype.h>
#include <string.h>

#include <sys/uio.h>

#include "convert.h"
#include "types.h"

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief advance slice segment
 * 
 * Get next binary element or zero-deliminated character part.
 * 
 * \param s  data slice descriptor
 * 
 * \return string segment type
 */
extern int mpt_slice_next(MPT_STRUCT(slice) *s)
{
	const MPT_STRUCT(buffer) *buf;
	const MPT_STRUCT(type_traits) *traits;
	const char *base, *end;
	ssize_t len;
	
	/* no remaining data */
	if (!(buf = s->_a._buf)
	  || (len = buf->_used - s->_off) < 0) {
		return MPT_ERROR(MissingData);
	}
	if (!(traits = buf->_content_traits) || !traits->size) {
		return MPT_ERROR(BadArgument);
	}
	/* consume existing segment */
	if (s->_len) {
		len -= s->_len;
		if (!len) {
			s->_off += s->_len;
			s->_len = 0;
			return 0;
		}
	}
	if (len < (ssize_t) traits->size) {
		return MPT_ERROR(MissingData);
	}
	/* alignment of data is required */
	if (traits->size != 1) {
		if (s->_off % traits->size) {
			return MPT_ERROR(BadValue);
		}
		len -= traits->size;
		if (len < 0) {
			return MPT_ERROR(MissingData);
		}
		s->_off += s->_len;
		s->_len = traits->size;
		return MPT_ENUM(TypeVector);
	}
	/* next byte of remaining data */
	if (traits != mpt_type_traits('c')) {
		s->_off += s->_len;
		s->_len = 1;
		return MPT_ENUM(TypeVector);
	}
	/* element base address and range */
	base = ((char *) (buf + 1)) + s->_off + s->_len;
	if ((end = memchr(base, 0, len))) {
		len = end + 1 - base;
	}
	s->_off += s->_len;
	s->_len = len;
	return end ? 's' : MPT_type_toVector('c');
}
