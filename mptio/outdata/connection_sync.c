/*!
 * finalize connection data
 */


#include <sys/uio.h>
#include <arpa/inet.h>

#include <poll.h>

#include "array.h"
#include "queue.h"
#include "event.h"

#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief syncronize connection
 * 
 * Wait for all sent messages to produce reply.
 * Interrupt (return -2) if non-reply message is received.
 * 
 * \param con      connection descriptor
 * \param timeout  time to wait for return messages
 * 
 * \return state of property
 */
extern int mpt_connection_sync(MPT_STRUCT(connection) *con, int timeout)
{
	struct pollfd sock;
	MPT_STRUCT(command) *ans;
	MPT_STRUCT(buffer) *buf;
	size_t len;
	int ret;
	
	if ((sock.fd = con->out.sock._id) < 0) {
		return MPT_ERROR(BadArgument);
	}
	sock.events = POLLIN;
	
	/* count waiting handlers */
	if (!(buf = con->_wait._buf) || !(len = buf->used)) {
		return 0;
	}
	len /= sizeof(*ans);
	
	for (ret = 0, ans = (void *) (buf + 1); len; --len, ++ans) {
		if (ans->cmd) {
			++ret;
		}
	}
	if (!(len = ret)) {
		return 0;
	}
	while (1) {
		uint16_t buf[64];
		struct iovec vec;
		
		vec.iov_base = buf;
		vec.iov_len = sizeof(buf);
		if (mpt_queue_peek(&con->in.data, &con->in.info, con->in.dec, &vec) >= 2) {
			ssize_t raw;
			
			buf[0] = ntohs(buf[0]);
			
			if (!(buf[0] & 0x8000)) {
				return MPT_ERROR(BadType);
			}
			if (!(ans = mpt_command_get(&con->_wait, buf[0] & 0x7fff))) {
				return MPT_ERROR(BadValue);
			}
			if ((raw = mpt_queue_recv(&con->in.data, &con->in.info, con->in.dec)) >= 0) {
				MPT_STRUCT(message) msg;
				size_t off;
				
				off = con->in.info.done;
				mpt_message_get(&con->in.data, off, raw, &msg, &vec);
				
				/* consume message ID */
				mpt_message_read(&msg, sizeof(*buf), 0);
				
				/* process reply and clear wait state */
				ret = ans->cmd(ans->arg, &msg);
				ans->cmd = 0;
				mpt_queue_crop(&con->in.data, 0, off);
				if (ret < 0) {
					return MPT_ERROR(BadOperation);
				}
				if (!--len) {
					return 0;
				}
				continue;
			}
		}
		if ((ret = poll(&sock, 1, timeout)) < 0) {
			break;
		}
		if (!(sock.revents & POLLIN)) {
			break;
		}
		
		if (con->in.data.len >= con->in.data.max && !mpt_queue_prepare(&con->in.data, 64)) {
			break;
		}
		if ((ret = mpt_queue_load(&con->in.data, con->out.sock._id, 0)) <= 0) {
			break;
		}
	}
	return len;
}
