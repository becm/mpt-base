/*!
 * set initial parameter for output descriptor
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "array.h"
#include "message.h"
#include "convert.h"
#include "meta.h"

#include "output.h"

/*!
 * \ingroup mptMessage
 * \brief print data to file
 * 
 * Print available data elements to file.
 * 
 * \param fd   file descriptor
 * \param hist history state information
 * \param len  length of data
 * \param src  start address of data
 */
extern ssize_t mpt_history_values(MPT_STRUCT(history) *hist, size_t len, const void *src)
{
	MPT_STRUCT(valfmt) *fmt;
	MPT_STRUCT(buffer) *buf;
	FILE *fd;
	const char *endl, *dat;
	size_t flen, dlen;
	ssize_t done;
	
	switch (hist->info.state & 0x3) {
	  case MPT_OUTFLAG(PrintNormal):
		fd = stdout;
		endl = mpt_newline_string(0);
		break;
	  case MPT_OUTFLAG(PrintError):
		fd = stderr;
		endl = mpt_newline_string(0);
		break;
	  case MPT_OUTFLAG(PrintHistory):
		if ((fd = hist->info.file)) {
			endl = mpt_newline_string(hist->info.lsep);
		} else {
			endl = mpt_newline_string(0);
			fd = stdout;
		}
		break;
	  default:
		if (!(fd = hist->info.file)) {
			return len;
		}
		endl = mpt_newline_string(hist->info.lsep);
	}
	/* finish data block */
	if (!len) {
		if (hist->fmt.pos) {
			len = strlen(endl);
			fwrite(endl, len, 1, fd);
			return len;
		}
		return 0;
	}
	if (!src) {
		return MPT_ERROR(BadOperation);
	}
	done = 0;
	
	/* indicate setup state */
	if (!hist->info.mode) {
		const int8_t *curr = src;
		
		if (*curr < 0) {
			return MPT_ERROR(BadValue);
		}
		/* separate type for data */
		if (!(hist->fmt.pos = *curr)) {
			hist->info.mode = 0x80;
		}
		hist->info.mode |= MPT_MESGTYPE(ValueFmt);
		src = curr + 1;
		++done;
		if (!--len) {
			return 1;
		}
	}
	dlen = 0;
	if ((buf = hist->fmt._dat._buf)) {
		dat = (void *) (buf + 1);
		dlen = buf->_used / sizeof(*dat);
	}
	/* require leading format identifiers */
	if (!(hist->info.mode & 0x80)
	    && !hist->fmt.fmt) {
		const int8_t *curr = src;
		
		while (dlen < hist->fmt.pos) {
			if (!(*curr & 0x7f)) {
				return MPT_ERROR(BadType);
			}
			if (!mpt_array_append(&hist->fmt._dat, sizeof(*curr), curr)) {
				return MPT_ERROR(BadOperation);
			}
			/* update data type info */
			dat = (void *) (buf + 1);
			++dlen;
			src = curr + 1;
			++done;
			
			if (!--len) {
				return done;
			}
		}
		/* format collection complete */
		hist->fmt.fmt = hist->fmt.pos;
		hist->fmt.pos = 0;
	}
	/* get otput format data */
	flen = 0;
	if ((buf = hist->fmt._fmt._buf)) {
		fmt = (void *) (buf + 1);
		flen = buf->_used / sizeof(*fmt);
	}
	while (len) {
		char buf[256];
		const char *curr = src;
		MPT_STRUCT(valfmt) val = MPT_VALFMT_INIT;
		int adv, conv;
		uint16_t pos;
		char cfmt = hist->fmt.fmt;
		
		/* use prepared format data */
		if (dlen) {
			if (hist->fmt.pos >= dlen) {
				fputs(endl, fd);
				hist->fmt.pos = 0;
			}
			cfmt = dat[hist->fmt.pos];
		}
		/* need current format information */
		else if ((hist->info.mode & 0x80) && !cfmt) {
			if (!(cfmt = *curr)) {
				return done ? done : MPT_ERROR(BadType);
			}
			hist->fmt.fmt = cfmt;
			src = ++curr;
			++done;
			
			if (!(--len)) {
				return done;
			}
		}
		if ((adv = mpt_msgvalfmt_size(cfmt)) < 0) {
			return done ? done : MPT_ERROR(BadType);
		}
		if (len < (size_t) adv) {
			return done ? done : MPT_ERROR(MissingData);
		}
		if ((adv > 1)
		    && ((cfmt & MPT_MESGVAL(ByteOrderLittle)) != MPT_MESGVAL(ByteOrderNative))) {
			int i;
			for (i = 0; i < adv; ++i) buf[adv-i] = curr[i];
			curr = buf;
			cfmt = (cfmt & ~MPT_MESGVAL(ByteOrderLittle)) | MPT_MESGVAL(ByteOrderNative);
		}
		if ((cfmt = mpt_msgvalfmt_type(cfmt)) < 0) {
			return done ? done : cfmt;
		}
		/* determine output format */
		if (!(hist->info.mode & 0x80)) {
			if (hist->fmt.pos < flen) {
				val = fmt[hist->fmt.pos];
			}
			/* reuse last format */
			else if (flen) {
				val = fmt[flen-1];
			}
		}
		/* print number to buffer */
		if ((conv = mpt_number_print(buf, sizeof(buf), val, cfmt, curr)) < 0) {
			return done ? done : conv;
		}
		/* consume format information */
		if (hist->info.mode & 0x80) {
			hist->fmt.fmt = 0;
		}
		/* stretch field size */
		if (conv < val.wdt) {
			conv = val.wdt;
		}
		/* field separation */
		if ((pos = hist->fmt.pos++)) {
			fputc(' ', fd);
		}
		fwrite(buf, conv, 1, fd);
		/* additional space on position overflow */
		if (!++pos) {
			fputc(' ', fd);
		}
		done += adv;
		len -= adv;
		
		src = ((uint8_t *) src) + adv;
	}
	return done;
}
