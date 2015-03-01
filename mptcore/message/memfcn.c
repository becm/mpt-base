/*!
 * find first/last occurance of accepted character.
 */

#include <limits.h>
#include <errno.h>

#include <sys/uio.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief get first position
 * 
 * Search first position of byte token.
 * 
 * \param data data arrays
 * \param ndat number of data arrays
 * \param fcn  token match function
 * \param par  match function argument
 * 
 * \return absolute position of token
 */
extern ssize_t mpt_memfcn(const struct iovec *data, size_t ndat, int (*fcn)(int , void *), void *par)
{
	size_t i = 0;
	
	if (!data || !fcn) {
		errno = EFAULT;
		return -1;
	}
	while (i < ndat) {
		unsigned char *tmp = data[i].iov_base;
		size_t pos = 0, len = data[i].iov_len;
		
		while (pos < len) {
			if (fcn(tmp[pos++], par)) {
				for (ndat = 0; ndat < i; ++ndat) {
					if ((pos += data[ndat].iov_len) > SSIZE_MAX) {
						errno = EOVERFLOW;
						return -1;
					}
				}
				return pos - 1;
			}
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
 * \param data data arrays
 * \param ndat number of data arrays
 * \param fcn  token match function
 * \param par  match function argument
 * 
 * \return absolute position of token
 */
extern ssize_t mpt_memrfcn(const struct iovec *data, size_t ndat, int (*fcn)(int , void *), void *par)
{
	size_t i = ndat;
	
	if (!data || !fcn) {
		errno = EFAULT; return -1;
	}
	
	while (i--) {
		unsigned char *tmp = data[i].iov_base;
		size_t pos = data[i].iov_len;
		
		while (pos--) {
			if (fcn(tmp[pos], par)) {
				for (ndat = 0; ndat < i; ++ndat) {
					if ((pos += data[ndat].iov_len) > SSIZE_MAX) {
						errno = EOVERFLOW;
						return -1;
					}
				}
				return pos;
			}
		}
	}
	
	errno = EAGAIN;
	
	return -2;
}

