/*!
 * copy memory with multiple source and target parts.
 */

#include <string.h>
#include <errno.h>

#include <sys/uio.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief copy data
 * 
 * Copy data between io-arrays.
 * 
 * \param len	size to copy
 * \param src	source data arrays
 * \param src	number of source arrays
 * \param dest	target data arrays
 * \param ndest	number of target arrays
 * 
 * \return copied size
 */
extern ssize_t mpt_memcpy(ssize_t len,
			  const struct iovec *src,  size_t nsrc,
			  const struct iovec *dest, size_t ndest)
{
	int8_t	*source, *target;
	size_t	total = 0, left, space;
	
	if (!ndest || !nsrc)
		return 0;
	
	source = src->iov_base;
	left   = src->iov_len;
	
	target = dest->iov_base;
	space  = dest->iov_len;
	
	/* check maximum size */
	if (len > 0) {
		size_t pos;
		for (total = left,  pos = 1; pos < nsrc;  ++pos) total += src[pos].iov_len;
		if (len > (ssize_t) total) return -1;
		for (total = space, pos = 1; pos < ndest; ++pos) total += dest[pos].iov_len;
		if (len > (ssize_t) total) return -2;
	}
	total = 0;
	
	while (len) {
		size_t copy = len;
		if (!left) {
			if (!(--nsrc)) break;
			++src;
			source = src->iov_base;
			left   = src->iov_len;
			continue;
		}
		if (!space) {
			if (!(--ndest)) break;
			++dest;
			target = dest->iov_base;
			space  = dest->iov_len;
			continue;
		}
		if (left  < copy) copy = left;
		if (space < copy) copy = space;
		
		memcpy(target, source, copy);
		
		source += copy;
		left   -= copy;
		
		target += copy;
		space  -= copy;
		
		total += copy;
		len   -= copy;
	}
	return total;
}
