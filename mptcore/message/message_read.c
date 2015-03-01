

#include <string.h>
#include <sys/uio.h>

#include "message.h"

/*!
 * \ingroup mptMessage
 * \brief read from mpt::message
 * 
 * read/consume leading data of message
 * 
 * \param msg  source message data
 * \param len  available length
 * \param dest target address
 * 
 * \return length of consumed data
 */
extern size_t mpt_message_read(MPT_STRUCT(message) *msg, size_t len, void *dest)
{
	size_t	total = 0, part;
	
	while (len > (part = msg->used)) {
		if (part) {
			if (dest) {
				memcpy(dest, msg->base, part);
				dest = ((uint8_t *) dest) + part;
			}
			total += part;
			len   -= part;
		}
		if (!msg->clen) {
			msg->base = ((uint8_t *) msg->base) + part;
			msg->used -= part;
			return total;
		}
		msg->used = msg->cont->iov_len;
		msg->base = msg->cont->iov_base;
		++msg->cont;
		--msg->clen;
	}
	if (dest) memcpy(dest, msg->base, len);
	
	msg->base = ((uint8_t *) msg->base) + len;
	msg->used -= len;
	
	while (!msg->used && msg->clen) {
		msg->used = msg->cont->iov_len;
		msg->base = msg->cont->iov_base;
		++msg->cont;
		--msg->clen;
	}
	
	return total + len;
}
/*!
 * \ingroup mptMessage
 * \brief length of mpt::message
 * 
 * calculate total length of message
 * 
 * \param msg	source message data
 * 
 * \return message length
 */
extern size_t mpt_message_length(const MPT_STRUCT(message) *msg)
{
	const struct iovec *vec;
	size_t	len, left;
	
	len  = msg->used;
	left = msg->clen;
	vec  = msg->cont;
	
	while (left--) len += (vec++)->iov_len;
	
	return len;
}
