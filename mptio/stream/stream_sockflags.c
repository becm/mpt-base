
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "queue.h"
#include "stream.h"

#include "array.h"
#include "message.h"

#include "convert.h"

#include "output.h"

/*!
 * \ingroup mptStream
 * \brief stream to socket flags
 * 
 * Convert socket connect return code to
 * stream open flags.
 * 
 * \param flg  socket connect flags
 * 
 * \return stream open flags
 */
extern int mpt_stream_sockflags(int flg)
{
	if (!(flg & MPT_ENUM(SocketStream))) {
		return -1;
	}
	switch (flg & MPT_ENUM(SocketRdWr)) {
	  case MPT_ENUM(SocketRead):
		return MPT_ENUM(StreamRead)  | MPT_ENUM(StreamBuffer);
	  case MPT_ENUM(SocketWrite):
		return MPT_ENUM(StreamWrite) | MPT_ENUM(StreamWriteBuf);
	  case MPT_ENUM(SocketRdWr):
		return MPT_ENUM(StreamRdWr)  | MPT_ENUM(StreamReadBuf);
	  default:
		return 0;
	}
}
