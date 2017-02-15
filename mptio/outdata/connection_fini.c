/*!
 * finalize connection data
 */

#include <stdlib.h>

#include "stream.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief close connection
 * 
 * Clear connection related data elements.
 * 
 * \param con  connection descriptor
 */
extern void mpt_connection_close(MPT_STRUCT(connection) *con)
{
	MPT_STRUCT(buffer) *buf;
	MPT_INTERFACE(reply_context) *rc;
	
	if (MPT_socket_active(&con->out.sock)) {
		mpt_outdata_close(&con->out);
	}
	else if ((buf = con->out.buf._buf)) {
		mpt_stream_close((void *) buf);
	}
	con->cid = 0;
	mpt_command_clear(&con->_wait);
	
	if ((rc = con->_rctx)) {
		rc->_vptr->ref.unref((void *) rc);
		con->_rctx = 0;
	}
}

/*!
 * \ingroup mptOutput
 * \brief finalize connection data
 * 
 * Clear allocations and resources in connection data
 * 
 * \param con  connection descriptor
 */
extern void mpt_connection_fini(MPT_STRUCT(connection) *con)
{
	MPT_STRUCT(buffer) *buf;
	
	mpt_connection_close(con);
	if (!MPT_socket_active(&con->out.sock)
	    && (buf = con->out.buf._buf)) {
		free(buf);
	}
	mpt_array_clone(&con->_wait, 0);
}
