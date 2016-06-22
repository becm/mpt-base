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
 * \brief send message via connection
 * 
 * Use connection to send message with registered ID.
 * 
 * \param con  connection descriptor
 * \param src  message to send
 */
extern int mpt_connection_send(MPT_STRUCT(connection) *con, const MPT_STRUCT(message) *src)
{
	ssize_t take;
	
	if (!(con->out.state & MPT_ENUM(OutputActive)) && con->out._idlen) {
		uint8_t buf[64];
		if (con->out._idlen > sizeof(buf)
		    || mpt_message_id2buf(buf, con->out._idlen, con->cid) < 0) {
			return MPT_ERROR(BadValue);
		}
		if (mpt_outdata_push(&con->out, con->out._idlen, buf) < 0) {
			return MPT_ERROR(BadOperation);
		}
		con->out.state |= MPT_ENUM(OutputActive);
	}
	
	if (src) {
		const uint8_t *base = src->base;
		size_t used = src->used;
		struct iovec *cont = src->cont;
		size_t clen = src->clen;
		
		while (1) {
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
			take = mpt_outdata_push(&con->out, used, base);
			
			if (take < 0 || (size_t) take > used) {
				if ((con->out.state & MPT_ENUM(OutputActive))
				    && (take = mpt_outdata_push(&con->out, 1, 0)) < 0) {
					return take;
				}
				return -1;
			}
			/* advance part data */
			con->out.state |= MPT_ENUM(OutputActive);
			base += take;
			used -= take;
		}
	}
	if ((take = mpt_outdata_push(&con->out, 0, 0)) < 0) {
		if ((con->out.state & MPT_ENUM(OutputActive))) {
			(void) mpt_outdata_push(&con->out, 1, 0);
		}
		take = -1;
	}
	con->out.state &= ~(MPT_ENUM(OutputActive) | MPT_ENUM(OutputRemote));
	
	return take;
}
