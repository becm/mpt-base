/*!
 * find first/last occurance of one of supplied  characters.
 */

#include <string.h>
#include <errno.h>

#include <sys/uio.h>

#include "message.h"

static int memchr_wrap(int curr, const struct iovec *match)
{
	return (memchr(match->iov_base, curr, match->iov_len)) ? 1 : 0;
}

/*!
 * \ingroup mptMessage
 * \brief get first position
 * 
 * Search first position of any byte token.
 * 
 * \param data	data arrays
 * \param ndat	number of data arrays
 * \param match	match tokens
 * \param mlen	number of tokens
 * 
 * \return absolute position of token
 */
extern ssize_t mpt_memstr(const struct iovec *data, size_t ndat, const void *match, size_t mlen)
{
	struct iovec	tmp;
	
	if ( !(tmp.iov_len = mlen) )
		return 0;
	
	if ( (tmp.iov_base = (void *) match) == NULL ) {
		errno = EFAULT; return -1;
	}
	return mpt_memfcn(data, ndat, (int (*)(int , void *)) memchr_wrap, &tmp);
}

/*!
 * \ingroup mptMessage
 * \brief get last position
 * 
 * Search last position of any byte token.
 * 
 * \param data	data arrays
 * \param ndat	number of data arrays
 * \param match	match tokens
 * \param mlen	number of tokens
 * 
 * \return absolute position of token
 */
extern ssize_t mpt_memrstr(const struct iovec *data, size_t ndat, const void *match, size_t mlen)
{
	struct iovec	tmp;
	
	if ( !(tmp.iov_len = mlen) )
		return 0;
	
	if ( (tmp.iov_base = (void *) match) == NULL ) {
		errno = EFAULT; return -1;
	}
	return mpt_memrfcn(data, ndat, (int (*)(int , void *)) memchr_wrap, &tmp);
}
