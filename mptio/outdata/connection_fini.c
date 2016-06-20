/*!
 * finalize connection data
 */

#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "queue.h"
#include "event.h"

#include "output.h"

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
	FILE *fd;
	
	mpt_outdata_fini(&con->out);
	mpt_command_clear(&con->_wait);
	
	free(con->in.data.base);
	
	mpt_array_clone(&con->hist.info._fmt, 0);
	
	if ((fd = con->hist.file)
	    && (fd != stdin)
	    && (fd != stdout)
	    && (fd != stderr)) {
		fclose(fd);
		con->hist.file = 0;
	}
	if ((buf = con->_ctx._buf)) {
		MPT_STRUCT(reply_context) **ctx = (void *) (buf+1);
		mpt_reply_clear(ctx, buf->used / sizeof(*ctx));
		mpt_array_clone(&con->_ctx, 0);
	}
}
