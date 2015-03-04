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
 * \brief send message
 * 
 * Push complete message and flush stream.
 * 
 * \param srm stream descriptor
 * \param m   message to push
 * 
 * \return zero on success
 */
extern int mpt_stream_send(MPT_STRUCT(stream) *srm, const MPT_STRUCT(message) *m)
{
	const struct iovec *cont;
	const uint8_t *base;
	size_t used, clen;
	
	if (!m) {
		if (mpt_stream_push(srm, 0, 0) < 0) {
			return -1;
		}
		(void) mpt_stream_flush(srm);
		return 0;
	}
	base = m->base;
	used = m->used;
	cont = m->cont;
	clen = m->clen;
	
	while (1) {
		ssize_t curr = mpt_stream_push(srm, used, base);
		
		if (curr < 0 || (size_t) curr > used) {
			if (mpt_stream_push(srm, 1, 0) < 0) {
				return -1;
			} else {
				return -2;
			}
		}
		if ((used -= curr)) {
			base += curr;
			continue;
		}
		if (!clen) {
			(void) mpt_stream_flush(srm);
			return 0;
		}
		base = cont->iov_base;
		used = cont->iov_len;
		
		++cont;
	}
}
