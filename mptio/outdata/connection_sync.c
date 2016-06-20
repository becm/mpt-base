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
		uint8_t buf[32];
		
		if (mpt_queue_peek(&con->in, sizeof(buf), buf) >= 2) {
			ssize_t raw;
			uint16_t id;
			
			if (!(buf[0] & 0x80)) {
				return MPT_ERROR(BadType);
			}
			buf[0] &= 0x7f;
			id = ntohs(*((uint16_t *) buf));
			
			if (!(ans = mpt_command_get(&con->_wait, id))) {
				return MPT_ERROR(BadValue);
			}
			if ((raw = mpt_queue_recv(&con->in)) >= 0) {
				MPT_STRUCT(message) msg;
				struct iovec vec;
				
				mpt_queue_crop(&con->in.data, 0, con->in._state.done);
				con->in._state.done = 0;
				mpt_message_get(&con->in.data, 0, raw, &msg, &vec);
				
				/* consume message ID */
				mpt_message_read(&msg, sizeof(*buf), 0);
				
				/* process reply and clear wait state */
				ret = ans->cmd(ans->arg, &msg);
				ans->cmd = 0;
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
