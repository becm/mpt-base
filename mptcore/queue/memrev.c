/*!
 * swap memory before and after pivot.
 */

#include <string.h>
#include <errno.h>

#include "queue.h"

extern void *mpt_memswap(void *from, void *to, size_t len)
{
	char buf[1024];
	size_t parts, i;
	
	if ( !from || !to ) {
		errno = EFAULT;
		return NULL;
	}
	
	parts = len / sizeof(buf);
	len   = len % sizeof(buf);
	
	for ( i = 0 ; i < parts ; i++ ) {
		(void) memcpy(buf, from, sizeof(buf));
		(void) memcpy(from, to, sizeof(buf));
		(void) memcpy(to, buf, sizeof(buf));
		
		from = ((char *) from) + sizeof(buf);
		to   = ((char *) to)   + sizeof(buf);
	}
	if ( len ) {
		(void) memcpy(buf, from, len);
		(void) memcpy(from, to, len);
		(void) memcpy(to, buf, len);
	}
	
	return from;
}

extern void *mpt_memrev(void *data, size_t pre, size_t len)
{
	size_t post;
	
	if ( !data ) {
		errno = EFAULT;
		return NULL;
	}
	if ( len < pre ) {
		errno = ERANGE;
		return NULL;
	}
	post = len - pre;
	
	while ( pre && post ) {
		if ( pre <= 1024 ) {
			char	tmp[1024];
			(void) memcpy(tmp, data, pre);
			(void) memmove(data, ((char *) data) + pre, post);
			(void) memcpy(((char *) data) + post, tmp, pre);
			break;
		}
		if ( post <= 1024 ) {
			char	tmp[1024];
			(void) memcpy(tmp, ((char *) data) + pre, post);
			(void) memmove(((char *) data) + post, data, pre);
			(void) memcpy(data, tmp, post);
			break;
		}
		/* swap lower part of memory */
		if ( pre < post ) {
			mpt_memswap(data, ((char *) data) + pre, pre);
			data = ((char *) data) + pre;
			post -= pre;
		}
		/* swap higher part of memory */
		else {
			len = pre - post;
			mpt_memswap(((char *) data) + len, ((char *) data) + pre, post);
			pre = len;
		}
	}
	
	return data;
}

