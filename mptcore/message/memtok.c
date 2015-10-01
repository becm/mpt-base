/*!
 * find next token (visible character) in data parts.
 * 
 * ignore escaped parts, stop on comments.
 */

#include <string.h>
#include <limits.h>
#include <ctype.h>
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
 * Ignore comments and allow 
 * 
 * \param data	data arrays
 * \param ndat	number of data arrays
 * \param tok	match tokens
 * \param com	comment tokens
 * \param esc	escape tokens
 * 
 * \return absolute position of token
 */
extern ssize_t mpt_memtok(const struct iovec *data, size_t ndat, const char *tok, const char *com, const char *escape)
{
	unsigned char	*curr = NULL;
	size_t	i = 0, len = 0, pos = 0, tlen, clen, elen;
	int	match = 0, prev = ' ';
	
	if ( data == NULL ) {
		errno = EFAULT; return -1;
	}
	
	tlen = tok ? strlen(tok) : 0;
	clen = com ? strlen(com) : 0;
	elen = escape ? strlen(escape) : 0;
	
	while ( 1 ) {
		/* continue in next data part */
		if ( ++pos >= len ) {
			if ( i >= ndat ) {
				errno = EAGAIN; return -2;
			}
			pos  = 0;
			curr = data[i].iov_base;
			if ( !(len = data[i++].iov_len ) )
				continue;
		}
		/* check for escape character */
		if ( elen ) {
			if ( match ) {
				/* unset if current is valid end */
				if ( *curr == match && prev != '\\' )
					match = 0;
				prev = *(curr++);
				continue;
			}
			/* mark if current is in delimiters */
			else if ( memchr(escape, *curr, elen) ) {
				match = *(curr++);
				continue;
			}
		}
		/* charater is in comments */
		if ( clen && memchr(com, *curr, clen) && isspace(prev) ) {
			if ( tok )
				break;
			/* continue until end of line */
			do {
				while ( pos++ < len && *(++curr) != '\n' );
				
				if ( pos <= len )
					break;
				else if ( i >= ndat ) {
					errno = EAGAIN; return -2;
				}
				pos  = 0;
				curr = data[i].iov_base;
				len  = data[i++].iov_len;
				
			} while ( 1 );
		}
		/* token is found */
		if ( tok ) {
			if ( tlen && memchr(tok, *curr, tlen) )
				break;
		}
		/* finding visible character succeeded */
		else if ( !isspace(*curr) )
			break;
		
		prev = *(curr++);
	}
	
	i--;
	
	/* escaped sequence unfinished */
	if ( match ) {
		errno = EAGAIN; return -2;
	}
	for ( elen = 0 ; elen < i ; elen ++ ) {
		/* offset is to big for return value */
		if ( (pos += data[elen].iov_len) > SSIZE_MAX ) {
			errno = EOVERFLOW; return -1;
		}
	}
	
	return pos;
}

