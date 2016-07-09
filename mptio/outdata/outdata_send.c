/*!
 * finalize connection data
 */

#include <sys/uio.h>
#include <arpa/inet.h>

#include "array.h"
#include "queue.h"

#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief send message via outdata
 * 
 * Use output data to send message.
 * 
 * \param out  outdata descriptor
 * \param src  message to send
 */
extern int mpt_outdata_send(MPT_STRUCT(outdata) *out, const MPT_STRUCT(message) *src)
{
	int ret = 0;
	
	if (src) {
		const uint8_t *base = src->base;
		size_t used = src->used;
		struct iovec *cont = src->cont;
		size_t clen = src->clen;
		
		while (1) {
			ssize_t curr;
			/* go to next non-empty part */
			if (!used) {
				if (!--clen) {
					break;
				}
				base = cont->iov_base;
				used = cont->iov_len;
				++cont;
				
				continue;
			}
			curr = mpt_outdata_push(out, used, base);
			
			if (curr < 0 || (size_t) curr > used) {
				if ((out->state & MPT_ENUM(OutputActive))
				    && (curr = mpt_outdata_push(out, 1, 0)) < 0) {
					return curr;
				}
				return -1;
			}
			/* advance part data */
			out->state |= MPT_ENUM(OutputActive);
			base += curr;
			used -= curr;
		}
	}
	if (mpt_outdata_push(out, 0, 0) < 0) {
		if ((out->state & MPT_ENUM(OutputActive))) {
			(void) mpt_outdata_push(out, 1, 0);
		}
		ret = -1;
	}
	out->state &= ~(MPT_ENUM(OutputActive) | MPT_ENUM(OutputRemote));
	
	return ret;
}
