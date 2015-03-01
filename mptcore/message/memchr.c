/*!
 * find first/last occurance of character in data parts.
 */

#include <string.h>
#include <limits.h>
#include <errno.h>

#include <sys/uio.h>

#include "message.h"

#ifndef SSIZE_MAX
# define SSIZE_MAX (SIZE_MAX/2)
#endif

/*!
 * \ingroup mptMessage
 * \brief get first position
 * 
 * Search first position of byte token.
 * 
 * \param data	data arrays
 * \param ndat	number of data arrays
 * \param tok	match token
 * 
 * \return absolute position of token
 */
extern ssize_t mpt_memchr(const struct iovec *data, size_t ndat, int tok)
{
	size_t	i = 0;
	
	while (ndat--) {
		const uint8_t *tmp;
		
		if (data[i].iov_len && (tmp = memchr(data[i].iov_base, tok, data[i].iov_len))) {
			ndat = tmp - ((uint8_t *) data[i].iov_base);
			
			while (i--) {
				if ((ndat += data[i].iov_len) > SSIZE_MAX) {
					errno = EOVERFLOW; return -1;
				}
			}
			
			return ndat;
		}
		i++;
	}
	
	errno = EAGAIN;
	
	return -2;
}
/*!
 * \ingroup mptMessage
 * \brief get last position
 * 
 * Search last position of byte token.
 * 
 * \param data	data arrays
 * \param ndat	number of data arrays
 * \param tok	match token
 * 
 * \return absolute position of token
 */
extern ssize_t mpt_memrchr(const struct iovec *data, size_t ndat, int tok)
{
	size_t	i = ndat;
	
	while (i--) {
		const uint8_t match = tok, *tmp = ((uint8_t *) data[i].iov_base) + data[i].iov_len;
		
		ndat = data[i].iov_len;
		
		while (ndat--) {
			if (*(--tmp) == match) {
				while (i) {
					if ((ndat += data[--i].iov_len) > SSIZE_MAX) {
						errno = EOVERFLOW; return -1;
					}
				}
				return ndat;
			}
		}
	}
	
	errno = EAGAIN;
	
	return -2;
}

