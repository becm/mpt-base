/*!
 * configure and prepare bound solver.
 */

#include <string.h>

#include <sys/uio.h>

#include "array.h"

#include "meta.h"

#include "convert.h"

/*!
 * \ingroup mptArray
 * \brief advance slice data
 * 
 * Consume slice data up to and including next NUL character.
 * 
 * \param s  data slice descriptor
 * 
 * \return length of consumed element
 */
extern ssize_t mpt_slice_advance(MPT_STRUCT(slice) *s)
{
	const MPT_STRUCT(buffer) *buf;
	const char *base, *end;
	size_t len;
	
	if (!(buf = s->_a._buf) || !s->_len) {
		return MPT_ERROR(MissingData);
	}
	base = (void *) (buf + 1);
	if (!(end = memchr(base + s->_off, 0, s->_len))) {
		return MPT_ERROR(MissingData);
	}
	len = (end + 1) - base;
	
	s->_off += len;
	s->_len -= len;
	
	return len;
}
