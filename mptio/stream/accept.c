/*!
 * accept connection to listening socket.
 */

#include <unistd.h>

#include <sys/socket.h>

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief new input on socket
 * 
 * Create input from new connection on
 * listenong socket.
 * 
 * \param socket listening socket descriptor
 * 
 * \return new input for connection
 */
extern MPT_INTERFACE(input) *mpt_accept(const MPT_STRUCT(socket) *socket)
{
	static int flags = MPT_ENUM(StreamRdWr) | MPT_ENUM(StreamBuffer);
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	MPT_INTERFACE(input) *in;
	int sockfd;
	
	if ((sockfd = accept(socket->_id, 0, 0)) < 0) {
		return 0;
	}
	sock._id = sockfd;
	
	/* bidirectional with 2 byte message ID */
	if (!(in = mpt_stream_input(&sock, flags, 0, sizeof(uint16_t)))) {
		(void) close(sockfd);
	}
	return in;
}
