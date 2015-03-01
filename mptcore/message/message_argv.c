/*!
 * get valid first argument from message payload.
 */

#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include <sys/uio.h>

#include "message.h"

static ssize_t nextChar(const struct iovec *curr, const struct iovec *cont, size_t clen, int match)
{
	ssize_t part;
	
	if ((part = mpt_memchr(curr, 1, match)) >= 0) {
		return part;
	}
	if (clen && (part = mpt_memchr(cont, clen, match)) >= 0) {
		return curr->iov_len + part;
	}
	part = curr->iov_len;
	
	while (clen--)
		part += (cont++)->iov_len;
	
	return part;
}
static int notSpace(int c, void *con)
{
	(void) con;
	return !isspace(c);
}

/*!
 * \ingroup mptMessage
 * \brief get next argument
 * 
 * Parse message for next argument end.
 * If separator is pace character
 * leading whitespace is consumed.
 * 
 * \param msg	message data
 * \param sep	argument separation character
 * 
 * \return length of next argument
 */
extern ssize_t mpt_message_argv(MPT_STRUCT(message) *msg, int sep)
{
	struct iovec *cont;
	struct iovec curr;
	ssize_t part;
	size_t  clen;
	
	/* start on base data */
	curr.iov_base = (void *) msg->base;
	while (!(curr.iov_len = msg->used)) {
		if (!msg->clen) {
			return 0;
		}
		curr.iov_base = (void *) (msg->base = msg->cont->iov_base);
		curr.iov_len = msg->used = msg->cont->iov_len;
		++msg->cont;
		--msg->clen;
	}
	cont = msg->cont;
	clen = msg->clen;
	
	/* find null character valid memory */
	if (!sep) {
		return nextChar(&curr, cont, clen, sep);
	}
	/* trim leading whitespace */
	if ((part = mpt_memfcn(&curr, 1, notSpace, 0)) >= 0) {
		msg->base = curr.iov_base = ((uint8_t *) curr.iov_base) + part;
		msg->used = curr.iov_len -= part;
	}
	else if ((part = mpt_memfcn(cont, clen, notSpace, 0)) >= 0) {
		while ((size_t) part > cont->iov_len) {
			part -= cont->iov_len;
			--clen;
			++cont;
		}
		msg->base = curr.iov_base = ((uint8_t *) cont->iov_base) + part;
		msg->used = curr.iov_len  = cont->iov_len - part;
		msg->cont = cont;
		msg->clen = clen;
	}
	/* find space character not in escapes */
	if (!isgraph(sep)) {
		if ((part = mpt_memtok(&curr, 1, "\t \n\r\v", NULL, "'\"")) >= 0) {
			return part;
		}
		if (clen && (part = mpt_memtok(cont, clen, "\t \n\r\v", NULL, "'\"")) >= 0) {
			return curr.iov_len + part;
		}
		sep = 0;
	}
	/* check for termination */
	return nextChar(&curr, cont, clen, sep);
}

