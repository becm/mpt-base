/*!
 * operations on streaminfo data
 */

#include <stdlib.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "stream.h"

#define _MPT_FD_BITS  (sizeof(uintptr_t)*8 - 16)

extern int mpt_stream_flags(const MPT_STRUCT(streaminfo) *info)
{
	return info->_fd & 0xffff;
}
extern void mpt_stream_seterror(MPT_STRUCT(streaminfo) *info, int err)
{
	info->_fd |= (err & 0xf) << 8;
}
extern void mpt_stream_clearerror(MPT_STRUCT(streaminfo) *info, int err)
{
	info->_fd = ((~err & 0xf) << 8) | (info->_fd & ~0xf00);
}
extern void mpt_stream_setnewline(MPT_STRUCT(streaminfo) *info, int nl, int what)
{
	switch (what & 0x3) {
	  case MPT_ENUM(StreamRead):  what = 0x1; break;
	  case MPT_ENUM(StreamWrite): what = 0x2; break;
	  default:                    what = 0x3;
	}
	nl &= 0x3;
	if (what & 0x1) {
		info->_fd = (nl << 12) | (info->_fd & ~(0x3 << 12));
	}
	if (what & 0x2) {
		info->_fd = (nl << 14) | (info->_fd & ~(0x3 << 14));
	}
}

extern int _mpt_stream_fwrite(const MPT_STRUCT(streaminfo) *info)
{
	int flags = mpt_stream_flags(info);
	
	if (flags & MPT_ENUM(StreamRdWr)) {
		if (flags & MPT_ENUM(StreamWrite))
			return (info->_fd >> (16 + _MPT_FD_BITS/2)) & ((1<<_MPT_FD_BITS/2) - 1);
		flags = info->_fd >> 16;
	}
	/* set to zero if NOT writable */
	else flags = (flags & MPT_ENUM(StreamWrite)) ? info->_fd >> 16 : 0;
	
	if (!flags) errno = EBADF;
	
	return flags - 1;
}

extern int _mpt_stream_fread(const MPT_STRUCT(streaminfo) *info)
{
	int flags = mpt_stream_flags(info);
	
	if (flags & MPT_ENUM(StreamRdWr)) {
		if (flags & MPT_ENUM(StreamWrite))
			return (info->_fd >> 16) & ((1<<_MPT_FD_BITS/2) - 1);
		flags = info->_fd >> 16;
	}
	/* set to zero if ONLY writable */
	else flags = (flags & MPT_ENUM(StreamWrite)) ? 0 : info->_fd >> 16;
	
	if (!flags) errno = EBADF;
	
	return flags - 1;
}

extern int _mpt_stream_setfile(MPT_STRUCT(streaminfo) *info, int nrfd, int nwfd)
{
	static const int flagmask = 0xfffc;
	static const intptr_t mx_fd = (1UL<<_MPT_FD_BITS)-2, mx_pfd = (1UL<<_MPT_FD_BITS/2)-1;
	int orfd, owfd;
	
	orfd = _mpt_stream_fread(info);
	owfd = _mpt_stream_fwrite(info);
	
	if (orfd == nrfd && owfd == nwfd) return 0;
	
	/* no read descriptor */
	if (nrfd < 0) {
		if (nwfd < 0) {
			info->_fd &= flagmask;
		}
		else if (nwfd >= mx_fd) {
			errno = EINVAL;
			return -1;
		}
		else {
			if (nwfd == orfd) orfd = -1;
			if (nwfd == owfd) owfd = -1;
			info->_fd &= flagmask;
			info->_fd |= MPT_ENUM(StreamWrite);
			info->_fd |= (nwfd+1) << 16;
		}
	}
	/* no write descriptor */
	else if (nwfd < 0) {
		if (nrfd >= mx_fd) {
			errno = EINVAL;
			return -1;
		}
		if (nrfd == orfd) orfd = -1;
		if (nrfd == owfd) owfd = -1;
		info->_fd &= flagmask;
		info->_fd |= MPT_ENUM(StreamRead);
		info->_fd |= (nrfd+1) << 16;
	}
	/* read/write descriptor */
	else if (nrfd == nwfd) {
		if (nrfd > mx_fd) {
			errno = EINVAL;
			return -1;
		}
		if (nrfd == orfd) orfd = -1;
		if (nrfd == owfd) owfd = -1;
		info->_fd &= flagmask;
		info->_fd |= MPT_ENUM(StreamRdWr);
		info->_fd |= (nrfd + 1) << 16;
	}
	/* read and write descriptors */
	else {
		if (nwfd > mx_pfd || nrfd > mx_pfd) {
			errno = EINVAL;
			return -1;
		}
		if (nrfd == orfd) orfd = -1;
		if (nrfd == owfd) owfd = -1;
		if (nwfd == orfd) orfd = -1;
		if (nwfd == owfd) owfd = -1;
		info->_fd &= flagmask;
		info->_fd |= MPT_ENUM(StreamWrite) | MPT_ENUM(StreamRdWr);
		info->_fd |= ((uintptr_t) nrfd) << 16;
		info->_fd |= ((uintptr_t) nwfd) << (_MPT_FD_BITS/2+16);
	}
	nwfd = 0;
	
	if (orfd >= 0) { close(orfd); nwfd |= 2; }
	if (owfd >= 0) { close(owfd); nwfd |= 1; }
	
	return nwfd;
}
