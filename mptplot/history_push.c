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
	if (hist->info.state & MPT_OUTFLAG(Active)) {
		/* local print condition */
		if (hist->info.state & 0x7) {
			return mpt_history_print(&hist->info, len, src);
		}
		/* history data output */
		ret = mpt_history_values(hist, len, src);
		if (ret < 0) {
			return ret;
		}
		if (!len) {
			hist->info.state &= ~MPT_OUTFLAG(Active);
			if (hist->info.file) {
				fflush(hist->info.file);
			}
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
	if (mt->cmd == MPT_MESGTYPE(ValueFmt)) {
		hist->info.state |= MPT_OUTFLAG(Active);
		if (!hist->info.file) {
			return 0;
		}
		ret = 0;
		if (--len
		    && (ret = mpt_history_values(hist, len, &mt->arg)) < 0) {
			return ret;
		}
		return ret + 1;
	}
	/* convert history to printable output */
	if (mt->cmd == MPT_MESGTYPE(ValueRaw)) {
		const MPT_STRUCT(msgbind) *mb = (const void *) (mt + 1);
		static const size_t min = sizeof(*mt) + sizeof(*mb);
		
		/* require message type and binding */
		if (len < min) {
			return MPT_ERROR(MissingData);
		}
		if (!(hist->fmt.fmt = mb->state)) {
			return MPT_ERROR(BadValue);
		}
		hist->info.state |= MPT_OUTFLAG(Active);
		hist->info.mode = MPT_MESGTYPE(ValueRaw);
		if (!hist->info.file) {
			return len;
		}
		ret = 0;
		if ((len -= min)
		    && (ret = mpt_history_values(hist, len, mt + 1)) < 0) {
			return ret;
		}
		return ret + min;
	}
	return mpt_history_print(&hist->info, len, src);
}
