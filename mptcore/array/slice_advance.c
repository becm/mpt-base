/*!
 * configure and prepare bound solver.
 */

#include <string.h>
#include <ctype.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief advance slice data
 * 
 * Set new offset for further elements and
 * reset current length.
 * 
 * \param s  data slice descriptor
 * 
 * \return next source type
 */
extern int mpt_slice_advance(MPT_STRUCT(slice) *s)
{
	const MPT_STRUCT(buffer) *buf;
	const char *base, *end;
	
	if (!(buf = s->_a._buf)) {
		return MPT_ERROR(MissingData);
	}
	if (s->_off >= buf->_used) {
		return MPT_ERROR(MissingData);
	}
	base = (void *) (buf + 1);
	/* advance converted area */
	if (s->_len) {
		size_t len = s->_off + s->_len;
		size_t left = buf->_used - len;
		/* consume tailing space and separator */
		while (left && isspace(base[len])) {
			--left;
			++len;
		}
		if (left && !base[len]) {
			++len;
		}
		/* no further data */
		if (len >= buf->_used) {
			s->_off = buf->_used;
			s->_len = 0;
			return 0;
		}
		s->_off = len;
		s->_len = 0;
		return 's';
	}
	/* find inline string separator */
	if (!(end = memchr(base + s->_off, 0, buf->_used - s->_off))) {
		s->_off = buf->_used;
		s->_len = 0;
		return 0;
	}
	s->_off = (end + 1) - base;
	s->_len = 0;
	
	return 's';
}
