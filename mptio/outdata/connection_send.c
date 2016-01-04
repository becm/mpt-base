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
	struct iovec vec;
	ssize_t take;
	uint16_t id;
	
	if (con->out.state & MPT_ENUM(OutputActive)) {
		return -2;
	}
	id = htons(con->cid);
	vec.iov_base = &id;
	vec.iov_len  = sizeof(id);
	
	if (mpt_array_push(&con->out._buf, &con->out._enc.info, con->out._enc.fcn, &vec) < (ssize_t) sizeof(id)) {
		return -1;
	}
	con->out.state |= MPT_ENUM(OutputActive);
	
	if (src) {
		struct iovec *cont = src->cont;
		size_t clen = src->clen;
		
		vec.iov_base = cont->iov_base;
		vec.iov_len  = cont->iov_len;
		
		while (1) {
			if (!vec.iov_len) {
				if (!--clen) {
					break;
				}
				vec = *(cont++);
				
				continue;
			}
			take = mpt_outdata_push(&con->out, vec.iov_len, vec.iov_base);
			
			if (take < 0 || (size_t) take > vec.iov_len) {
				if ((take = mpt_outdata_push(&con->out, 1, 0)) < 0) {
					return take;
				}
				return -1;
			}
			vec.iov_base = ((uint8_t *) vec.iov_base) + take;
			vec.iov_len -= take;
			
			/* mark written data */
			id = 0;
		}
	}
	if ((take = mpt_outdata_push(&con->out, 0, 0)) < 0) {
		if (!id) {
			(void) mpt_outdata_push(&con->out, 1, 0);
		}
		take = -1;
	}
	con->out.state &= ~(MPT_ENUM(OutputActive) | MPT_ENUM(OutputRemote));
	
	return take;
}
