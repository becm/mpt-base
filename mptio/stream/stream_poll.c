/*!
 * wait for IO on stream.
 */

#include <errno.h>

#include <poll.h>

#include "queue.h"
#include "output.h"

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief wait for stream operation
 * 
 * Wait until requested operation is applied
 * on stream descriptor.
 * 
 * \param srm     stream descriptor
 * \param what    type of operation(s) to wait for
 * \param timeout time to wait for operation
 * 
 * \retval >0 waiting message or output data
 * \retval 0  default value
 * \retval <0 error
 */
extern int mpt_stream_poll(MPT_STRUCT(stream) *srm, int what, int timeout)
{
	struct pollfd fd[2];
	int flags, keep;
	
	flags = mpt_stream_flags(&srm->_info);
	
	if (!(flags & MPT_STREAMFLAG(ReadBuf))) {
		what &= ~POLLIN;
	}
	if (!(flags & MPT_STREAMFLAG(WriteBuf))) {
		what &= ~POLLOUT;
	}
	/* fast path for available input */
	if (timeout < 0 && what == POLLIN) {
		fd[0].fd = _mpt_stream_fread(&srm->_info);
		fd[0].revents = what;
		fd[1].fd = -1;
	}
	/* fast path for output */
	else if (timeout < 0 && what == POLLOUT) {
		fd[0].fd = -1;
		fd[0].revents = 0;
		fd[1].fd = _mpt_stream_fread(&srm->_info);
		fd[1].revents = what;
	}
	/* wait for inputs */
	else {
		int shared;
		fd[0].revents = fd[1].revents = 0;
		
		fd[0].fd = _mpt_stream_fread(&srm->_info);
		fd[1].fd = _mpt_stream_fwrite(&srm->_info);
		
		fd[0].events = fd[0].fd < 0 ? 0 : (what & ~POLLOUT);
		fd[1].events = fd[1].fd < 0 ? 0 : (what & ~POLLIN);
		
		if ((shared = (fd[0].fd == fd[1].fd))) {
			if (fd[0].fd < 0) {
				errno = EAGAIN;
				return -2;
			}
			fd[0].events |= fd[1].events;
		}
		if ((keep = poll(fd[0].fd < 0 ? fd+1 : fd, shared || fd[0].fd < 0 ? 1 : 2, timeout)) < 0) {
			return keep;
		}
		if (shared) fd[0].revents |= fd[1].revents;
	}
	keep = -3;
	
	if (fd[0].fd >= 0) {
		ssize_t len;
		
		mpt_queue_shift(&srm->_rd);
		
		/* avoid removal if input available */
		if (!(fd[0].revents & POLLIN)) {
			if (srm->_rd._state.data.msg >= 0) {
				keep = what & POLLIN;
			}
		}
		/* prepare input buffer */
		else if ((srm->_rd.data.len == srm->_rd.data.max)
		         && ((flags & MPT_STREAMFLAG(WriteMap)) || !mpt_queue_prepare(&srm->_rd.data, 64))) {
			mpt_stream_seterror(&srm->_info, MPT_ENUM(ErrorRead));
		}
		/* read further input data */
		else if ((len = mpt_queue_load(&srm->_rd.data, fd[0].fd, 0)) > 0) {
			mpt_stream_clearerror(&srm->_info, MPT_ENUM(ErrorEmpty) | MPT_ENUM(ErrorRead));
			keep = POLLIN;
		}
		/* regular close */
		else if (!len) {
			fd[0].revents |= POLLHUP;
		}
		/* irregular operation */
		else {
			mpt_stream_seterror(&srm->_info, (fd[0].revents & POLLHUP) ? MPT_ENUM(ErrorEmpty) : MPT_ENUM(ErrorRead));
		}
		/* all available data reveived */
		if (fd[0].revents & POLLHUP
		    && !(fd[0].revents & POLLIN)) {
			mpt_stream_seterror(&srm->_info, MPT_ENUM(ErrorRead));
		}
	}
	else if (srm->_rd.data.len) {
		keep = 0;
	}
	if (fd[1].fd >= 0) {
		/* avoid removal if input not queried */
		if (!(fd[0].revents & POLLOUT)) {
			if (srm->_wd._state.done && keep < 0) {
				keep = 0;
			}
		}
		/* remaining data on output */
		else {
			int ret;
			
			if ((ret = mpt_stream_flush(srm)) < 0) {
				return keep;
			}
			if (ret) {
				if (keep < 0) keep = POLLOUT;
				else keep |= POLLOUT;
			}
			mpt_stream_clearerror(&srm->_info, MPT_ENUM(ErrorFull) | MPT_ENUM(ErrorWrite));
		}
	}
	return keep;
}
