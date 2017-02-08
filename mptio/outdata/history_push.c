/*!
 * finalize connection data
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "event.h"
#include "message.h"
#include "array.h"

#include "stream.h"

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
		/* history data output */
		ret = mpt_history_print(hist, len, src);
		if (ret < 0) {
			return ret;
		}
		if (!len) {
			hist->state &= ~MPT_ENUM(OutputActive);
		}
		return ret;
	}
	/* reset history state */
	mpt_history_reset(&hist->info);
	
	if (!len) {
		if (hist->file) {
			fputs(mpt_newline_string(hist->lsep), hist->file);
		}
		return 0;
	}
	mt = src;
	/* use inline or prefix format info */
	if (mt->cmd == MPT_ENUM(MessageValFmt)) {
		hist->state |= MPT_ENUM(OutputActive);
		if (!hist->file) {
			return 0;
		}
		ret = 0;
		if (--len
		    && (ret = mpt_history_print(hist, len, &mt->arg)) < 0) {
			return ret;
		}
		return ret + 1;
	}
	if (len < 2) {
		return MPT_ERROR(MissingData);
	}
	/* convert history to printable output */
	if (mt->cmd == MPT_ENUM(MessageValRaw)) {
		if (!(hist->info.all = mt->arg)) {
			return MPT_ERROR(BadValue);
		}
		if (!hist->file) {
			hist->state |= MPT_ENUM(OutputActive);
			return len;
		}
		hist->info.fmt = hist->info.all;
		ret = 0;
		if ((len -= 2)
		    && (ret = mpt_history_print(hist, len, mt + 1)) < 0) {
			return ret;
		}
		return ret + 2;
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
	/* incompatible message type */
	else {
		return MPT_ERROR(BadType);
	}
	flags = mpt_outdata_type(type, flags);
	
	if (!flags) {
		flags = MPT_ENUM(OutputPrintRestore);
	}
	hist->state = (hist->state & ~0x7) | (flags & 0x7);
	
	return mpt_file_print(&hist->state, hist->file, len, src);
}
