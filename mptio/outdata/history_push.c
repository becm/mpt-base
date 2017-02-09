/*!
 * finalize connection data
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "event.h"
#include "message.h"
#include "array.h"

#include "convert.h"

#include "output.h"

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
	ssize_t ret;
	
	/* message in progress */
	if (hist->info.state & MPT_ENUM(OutputActive)) {
		/* local print condition */
		if (hist->info.state & 0x7) {
			return mpt_history_print(&hist->info, len, src);
		}
		/* history data output */
		ret = mpt_history_values(&hist->info, &hist->fmt, len, src);
		if (ret < 0) {
			return ret;
		}
		if (!len) {
			hist->info.state &= ~MPT_ENUM(OutputActive);
		}
		return ret;
	}
	/* reset history state */
	mpt_histfmt_reset(&hist->fmt);
	hist->info.mode = 0;
	
	if (!len) {
		if (hist->info.file) {
			fputs(mpt_newline_string(hist->info.lsep), hist->info.file);
		}
		return 0;
	}
	mt = src;
	/* use inline or prefix format info */
	if (mt->cmd == MPT_ENUM(MessageValFmt)) {
		hist->info.state |= MPT_ENUM(OutputActive);
		if (!hist->info.file) {
			return 0;
		}
		ret = 0;
		if (--len
		    && (ret = mpt_history_values(&hist->info, &hist->fmt, len, &mt->arg)) < 0) {
			return ret;
		}
		return ret + 1;
	}
	if (len < 2) {
		return MPT_ERROR(MissingData);
	}
	/* convert history to printable output */
	if (mt->cmd == MPT_ENUM(MessageValRaw)) {
		if (!(hist->fmt.all = mt->arg)) {
			return MPT_ERROR(BadValue);
		}
		if (!hist->info.file) {
			hist->info.state |= MPT_ENUM(OutputActive);
			return len;
		}
		hist->fmt.fmt = hist->fmt.all;
		ret = 0;
		if ((len -= 2)
		    && (ret = mpt_history_values(&hist->info, &hist->fmt, len, mt + 1)) < 0) {
			return ret;
		}
		return ret + 2;
	}
	return mpt_history_print(&hist->info, len, src);
}
