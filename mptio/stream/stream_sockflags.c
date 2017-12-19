
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
	if (!(flg & MPT_SOCKETFLAG(Stream))) {
		return -1;
	}
	switch (flg & MPT_SOCKETFLAG(RdWr)) {
	  case MPT_SOCKETFLAG(Read):
		return MPT_STREAMFLAG(Read)  | MPT_STREAMFLAG(Buffer);
	  case MPT_SOCKETFLAG(Write):
		return MPT_STREAMFLAG(Write) | MPT_STREAMFLAG(WriteBuf);
	  case MPT_SOCKETFLAG(RdWr):
		return MPT_STREAMFLAG(RdWr)  | MPT_STREAMFLAG(Buffer);
	  default:
		return 0;
	}
}
