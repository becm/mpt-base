/*!
 * add message to array
 */

#include <sys/uio.h>

#include "array.h"

#include "message.h"

/*!
 * \ingroup mptOutput
 * \brief push message to array
 * 
 * Append complete message data after active array content.
 * 
 * \param arr  array descriptor
 * \param msg  message data to add
 */
extern int mpt_message_append(MPT_STRUCT(array) *arr, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(buffer) *buf;
	struct iovec *cont;
	size_t used ,olen, clen;
	
	olen = (buf = arr->_buf) ? buf->used : 0;
	
	/* process first message part */
	if ((used = msg->used)
	    && !mpt_array_append(arr, used, msg->base)) {
		/* reset array state */
		if ((buf = arr->_buf)) {
			buf-> used = olen;
		}
		return MPT_ERROR(MissingBuffer);
	}
	/* trailing message parts */
	cont = msg->cont;
	clen = msg->clen;
	while (--clen) {
		const uint8_t *base;
		base = cont->iov_base;
		used = cont->iov_len;
		++cont;
		/* skip empty segments */
		if (!used || mpt_array_append(arr, used, base)) {
			continue;
		}
		/* reset array state */
		if ((buf = arr->_buf)) {
			buf-> used = olen;
		}
		return MPT_ERROR(MissingBuffer);
	}
	return 0;
}
