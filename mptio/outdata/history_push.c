/*!
 * finalize connection data
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "event.h"
#include "message.h"
#include "array.h"

#include "output.h"

#define AnswerFlags(a) ((a & 0xf0) >> 4)
#define OutputFlags(a) (a & 0xf)

static int answerType(int code)
{
	if (!code) {
		return MPT_LOG(Debug);
	}
	else if (code < 0) {
		return MPT_LOG(Error);
	}
	return MPT_LOG(Info);
}
/*!
 * \ingroup mptOutput
 * \brief push to history
 * 
 * Append data to history descriptor or pass to next output.
 * 
 * \param hist history descriptor
 * \param len  size of new data
 * \param src  data to add
 * 
 * \return output descriptor
 */
extern ssize_t mpt_history_push(MPT_STRUCT(history) *hist, size_t len, const void *src)
{
	const MPT_STRUCT(msgtype) *mt;
	int type, flags;
	ssize_t ret;
	
	/* message in progress */
	if (hist->state & MPT_ENUM(OutputActive)) {
		/* local print condition */
		if (hist->state & 0x7) {
			return mpt_file_print(&hist->state, hist->file, len, src);
		}
		/* history output triggered */
		else if (hist->info.lfmt) {
			ret = mpt_history_print(hist->file, &hist->info, len, src);
		}
		/* pass data to chained output */
		else {
			ret = len;
			if (hist->pass) {
				ret = hist->pass->_vptr->push(hist->pass, len, src);
			}
		}
		if (ret < 0) {
			return ret;
		}
		if (!len) {
			hist->state &= MPT_ENUM(OutputPrintColor);
			hist->info.lfmt = 0;
		}
		return ret;
	}
	if (!src) {
		if (hist->pass) {
			return hist->pass->_vptr->push(hist->pass, len, src);
		}
		return MPT_ERROR(BadOperation);
	}
	mt = src;
	
	/* pass data to next output descriptot */
	if (hist->state & MPT_ENUM(OutputRemote)) {
		if (!hist->pass) {
			mpt_log(0, __func__, MPT_LOG(Info), "%s", MPT_tr("missing remote output"));
			return MPT_ERROR(BadArgument);
		}
		ret = hist->pass->_vptr->push(hist->pass, len, src);
		if (ret >= 0) {
			hist->state |= MPT_ENUM(OutputActive);
		}
		return ret;
	}
	/* convert history to printable output */
	if (mt->cmd == MPT_ENUM(MessageValFmt)) {
		/* reset history state */
		mpt_history_reset(&hist->info);
		hist->info.lfmt = -1;
		
		/* assume block termination */
		if (len < 2) {
			hist->info.pos.elem = 1;
			hist->state |= MPT_ENUM(OutputActive);
			return 1;
		}
		hist->info.pos.fmt = mt->arg;
		
		if (!hist->file) {
			hist->state |= MPT_ENUM(OutputActive);
			return 0;
		}
		ret = 0;
		if ((len -= 2)
		    && (ret = mpt_history_print(hist->file, &hist->info, len, mt+1)) < 0) {
			return ret;
		}
		hist->state |= MPT_ENUM(OutputActive);
		
		return ret + 2;
	}
	if (len < 2) {
		return MPT_ERROR(MissingData);
	}
	/* setup text output */
	if (mt->cmd == MPT_ENUM(MessageOutput)) {
		if (len < 2) {
			return MPT_ERROR(MissingData);
		}
		type  = mt->arg;
		flags = OutputFlags(hist->level);
	}
	else if (mt->cmd == MPT_ENUM(MessageAnswer)) {
		if (len < 2) {
			return MPT_ERROR(MissingData);
		}
		type  = answerType(mt->arg);
		flags = AnswerFlags(hist->level);
	}
	else if (hist->pass) {
		if ((ret = hist->pass->_vptr->push(hist->pass, len, src)) >= 0) {
			hist->state |= MPT_ENUM(OutputActive);
		}
		return ret;
	}
	else {
		mpt_log(0, __func__, MPT_LOG(Info), "%s: 0x%02d", MPT_tr("unsupported message type"), mt->cmd);
		hist->state |= MPT_ENUM(OutputActive);
		return len;
	}
	flags = mpt_outdata_type(type, flags);
	
	if (!flags) {
		flags = MPT_ENUM(OutputPrintRestore);
	}
	hist->state = (hist->state & ~0x7) | (flags & 0x7);
	
	return mpt_file_print(&hist->state, hist->file, len, src);
}
