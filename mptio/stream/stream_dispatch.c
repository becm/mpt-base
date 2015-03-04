/*!
 * stream message dispatching.
 */

#include <string.h>
#include <errno.h>

#include <sys/uio.h>

#include "message.h"
#include "convert.h"
#include "event.h"
#include "queue.h"

#include "stream.h"

/* check for non-zero ID */
static ssize_t nonZeroID(MPT_STRUCT(message) *msg, size_t idlen)
{
	struct iovec *cont = msg->cont;
	const uint8_t *base = msg->base;
	size_t used = msg->used, clen = msg->clen;
	ssize_t pos = 0;
	
	while (idlen) {
		size_t i;
		if (used > idlen) {
			used = idlen;
		}
		for (i = 0; i < used; ++i) {
			if (base[i]) {
				return pos;
			}
			++pos;
		}
		if (!(idlen -= used)) {
			return -2;
		}
		if (!clen--) {
			return -1;
		}
		base = cont->iov_base;
		used = cont->iov_len;
		++cont;
	}
	return -2;
}

/*!
 * \ingroup mptStream
 * \brief dispatch next message
 * 
 * Call message dispatcher with next message
 * in input queue.
 * 
 * \param srm   stream descriptor
 * \param idlen message identifier length
 * \param cmd   command handler
 * \param arg   argument for command handler
 * 
 * \return created input
 */
extern int mpt_stream_dispatch(MPT_STRUCT(stream) *srm, size_t idlen, MPT_TYPE(EventHandler) cmd, void *arg)
{
	struct iovec vec;
	MPT_STRUCT(message) msg;
	MPT_STRUCT(msgindex) idx;
	MPT_STRUCT(event) ev;
	ssize_t raw;
	int ret;
	
	if (idlen && !srm->_dec.fcn) {
		return -1;
	}
	
	/* message in progress */
	if (mpt_stream_flags(&srm->_info) & MPT_ENUM(StreamMesgAct)) {
		errno = EAGAIN;
		return -2;
	}
	if ((raw = srm->_dec.mlen) < 0
	    && (srm->_dec.mlen = raw = mpt_queue_recv(&srm->_rd, &srm->_dec.info, srm->_dec.fcn)) < 0) {
		return -2;
	}
	idx.len = raw;
	idx.off = raw = srm->_dec.info.done;
	srm->_dec.info.done = 0;
	mpt_message_get(&srm->_rd, &idx, &msg, &vec);
	
	ev.id = 0;
	ev.msg = &msg;
	ev.reply.set = 0;
	ev.reply.context = 0;
	
	/* setup reply channel */
	if (idlen) {
		MPT_STRUCT(message) tmp;
		uint8_t rbuf[sizeof(ev.id)];
		size_t pos = sizeof(rbuf);
		
		if (pos > idlen) {
			pos = idlen;
		}
		else if (mpt_message_length(&msg) < idlen) {
			mpt_queue_crop(&srm->_rd, 0, raw);
			return -1;
		}
		tmp = msg;
		/* read (first part of) message id */
		if (mpt_message_read(&msg, pos, rbuf) < pos) {
			mpt_queue_crop(&srm->_rd, 0, raw);
			return -1;
		}
		/* marked as reply */
		if (rbuf[0] & 0x80) {
			/* id fits event size */
			if (pos <= sizeof(ev.id)) {
				size_t i;
				ev.id = rbuf[0] & 0x7f;
				for (i = 1; i < pos; ++i) {
					ev.id = (ev.id * 0x100) + rbuf[pos];
				}
			}
			/* use pseudo-id */
			else {
				ev.id = -1;
				if (idlen > pos) {
					mpt_message_read(&msg, idlen - pos, 0);
				}
			}
			/* message was a reply */
			idlen = 0;
		}
		/* no reply needed */
		else if (!srm->_enc.fcn || (nonZeroID(&tmp, idlen) < 0)) {
			/* consume remaining id */
			if (idlen > pos) {
				mpt_message_read(&msg, idlen - pos, 0);
			}
			idlen = 0;
		}
		/* reply setup */
		else {
			rbuf[0] |= 0x80;
			ev.reply.set = (int (*)()) mpt_stream_send;
			ev.reply.context = srm;
			
			/* push reply id */
			while (1) {
				if (mpt_stream_push(srm, pos, rbuf) < 0) {
					mpt_queue_crop(&srm->_rd, 0, raw);
					return -3;
				}
				raw -= pos;
				if (!(idlen -= pos)) {
					break;
				}
				pos = (idlen > sizeof(rbuf)) ? sizeof(rbuf) : idlen;
				if (mpt_message_read(&msg, pos, rbuf) < pos) {
					MPT_ABORT("invalid ID processing");
				}
			}
		}
	}
	/* dispatch data to command */
	if (!cmd) {
		ret = 0;
	}
	else if ((ret = cmd(arg, &ev)) < 0) {
		ret = MPT_ENUM(EventCtlError);
	} else {
		ret &= MPT_ENUM(EventFlags);
	}
	/* remove message data from queue */
	mpt_queue_crop(&srm->_rd, 0, raw);
	
	/* finalize reply */
	if (idlen) {
		if (mpt_stream_flags(&srm->_info) & MPT_ENUM(StreamMesgAct)) {
			mpt_stream_push(srm, 0, 0);
		}
		mpt_stream_flush(srm);
	}
	
	/* further message on queue */
	if ((srm->_dec.mlen = mpt_queue_recv(&srm->_rd, &srm->_dec.info, srm->_dec.fcn)) >= 0) {
		ret |= MPT_ENUM(EventRetry);
	}
	return ret;
}
