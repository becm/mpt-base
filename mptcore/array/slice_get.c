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
 * \brief get values from slice
 * 
 * Convert zero-deliminated character data on slice to text elements.
 * 
 * \param s     data slice descriptor
 * \param type  target type code
 * \param data  value address
 * 
 * \return source type and applied flags
 */
extern int mpt_slice_get(MPT_STRUCT(slice) *s, int type, void *data)
{
	const MPT_STRUCT(buffer) *buf;
	const char *base, *end;
	ssize_t len;
	
	if (!type) {
		static const char types[] = { MPT_type_toVector('c'), 's', 0 };
		if (data) *((const char **) data) = types;
		return 0;
	}
	/* no remaining data */
	if (!(buf = s->_a._buf)
	    || (len = buf->_used - s->_off) <= 0) {
		return 0;
	}
	if (!buf->_content_traits
	 || (buf->_content_traits != mpt_type_traits(MPT_type_toVector('c')))) {
		return MPT_ERROR(BadType);
	}
	/* element base address and range */
	base = ((char *) (buf + 1)) + s->_off;
	if ((end = memchr(base, 0, len))) {
		len = end + 1 - base;
	}
	
	if (type == MPT_type_toVector('c')
	 || type == MPT_ENUM(TypeVector)) {
		struct iovec *vec;
		if ((vec = data)) {
			vec->iov_base = (char *) base;
			vec->iov_len  = len;
		}
		s->_len = len;
		return len ? type : MPT_ENUM(TypeArray);
	}
	/* types require string termination */
	if (!end) {
		return MPT_ERROR(MissingData);
	}
	
	if (type == 's') {
		if (data) {
			*((const char **) data) = base;
		}
		s->_len = len;
		return MPT_type_toVector('c');
	}
	if (type == 'k') {
		const char *key;
		end = base;
		if (!(key = mpt_convert_key(&end, 0, 0))) {
			return MPT_ERROR(BadType);
		}
		/* advance empy space and separator */
		while (isspace(*end)) {
			++end;
		}
		if (*end) {
			++end;
		}
		if (data) {
			*((const char **) data) = key;
		}
		s->_len = len;
		return MPT_type_toVector('c');
	}
	if ((len = mpt_convert_string(base, type, data)) <= 0) {
		return len;
	}
	/* consume trailing space */
	while (((size_t) len < s->_len) && isspace(base[len])) {
		++len;
	}
	/* include terminator if all data is covered */
	if (((size_t)(len + 1) == s->_len)) {
		s->_len = len + 1;
		return 's';
	}
	s->_len = len;
	return MPT_type_toVector('c');
}
