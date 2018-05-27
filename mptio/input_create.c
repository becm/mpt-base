/*!
 * create control connection
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "output.h"
#include "object.h"
#include "convert.h"

#include "stream.h"
#include "connection.h"

#include "notify.h"

/*!
 * \ingroup mptNotify
 * \brief create input
 * 
 * Determine input type and parameters from connection string.
 * 
 * \param ctl  remote endpoint
 * 
 * \return new input instance reference
 */
extern MPT_INTERFACE(input) *mpt_input_create(char const *ctl)
{
	MPT_INTERFACE(input) *in;
	int fd = -1;
	
	/* use stdin */
	if (ctl[0] == '-' && !ctl[1]) {
		if ((fd = dup(STDIN_FILENO)) < 0) {
			return 0;
		}
	}
	/* determine mode */
	else {
		MPT_STRUCT(fdmode) sm;
		int pos;
		if ((pos = mpt_mode_parse(&sm, ctl)) < 0) {
			errno = EINVAL;
			return 0;
		}
		if (!sm.family
		    && !MPT_stream_writable(sm.stream)
		    && (fd = open(ctl + pos, sm.param.file.open)) < 0) {
			return 0;
		}
	}
	/* bidirectional input */
	if (fd >= 0) {
		MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
		int mode = MPT_STREAMFLAG(Read) | MPT_STREAMFLAG(Buffer);
		sock._id = fd;
		if (!(in = mpt_stream_input(&sock, mode, MPT_ENUM(EncodingCommand), 0))) {
			close(fd);
		}
		return in;
	}
	if ((in = mpt_output_remote())) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		MPT_INTERFACE(object) *obj = 0;
		
		val.ptr = ctl;
		
		if (in->_vptr->meta.conv((void *) in, MPT_ENUM(TypeObject), &obj) < 0
		    || !obj
		    || mpt_object_set_value(obj, 0, &val) < 0) {
			in->_vptr->meta.ref.unref((void *) in);
			errno = EINVAL;
			return 0;
		}
	}
	return in;
}

