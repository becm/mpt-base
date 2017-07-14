/*!
 * stream reply
 */

#include <sys/uio.h>

#include "message.h"
#include "event.h"
#include "queue.h"

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief add message data
 * 
 * Push complete message content to stream.
 * 
 * \param srm stream descriptor
 * \param m   message to push
 * 
 * \return number of bytes written or error
 */
extern ssize_t mpt_stream_append(MPT_STRUCT(stream) *srm, const MPT_STRUCT(message) *m)
{
	const struct iovec *cont;
	const uint8_t *base;
	size_t used, clen, total;
	
	if (!m) {
		return 0;
	}
	base = m->base;
	used = m->used;
	cont = m->cont;
	clen = m->clen;
	
	total = 0;
	while (1) {
		ssize_t curr = mpt_stream_push(srm, used, base);
		
		if (curr < 0) {
			return MPT_ERROR(BadOperation);
		}
		total += curr;
		if ((used -= curr)) {
			base += curr;
			continue;
		}
		if (!clen) {
			return total;
		}
		base = cont->iov_base;
		used = cont->iov_len;
		
		++cont;
	}
}
