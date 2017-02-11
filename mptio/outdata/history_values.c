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

#include "stream.h"

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
	
	if (!(fd = hist->info.file)) {
		if (len && !src) {
			return MPT_ERROR(BadOperation);
		}
		return len;
	}
	endl = mpt_newline_string(hist->info.lsep);
	
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
		if (!(hist->info.mode = *curr)) {
			hist->info.mode = 0x80;
		} else {
			hist->fmt.pos = hist->info.mode;
		}
		src = curr + 1;
		++done;
		if (!--len) {
			return 1;
		}
	}
	dlen = 0;
	if ((buf = hist->fmt._dat._buf)) {
		dat = (void *) (buf + 1);
		dlen = buf->used / sizeof(*dat);
	}
	/* require leading format identifiers */
	if (!(hist->info.mode & 0x80)
	    && !(hist->fmt.fmt & 0x7f)) {
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
		flen = buf->used / sizeof(*fmt);
	}
	while (len) {
		char buf[256];
		const char *curr = src;
		MPT_STRUCT(valfmt) val = MPT_VALFMT_INIT;
		int adv, conv;
		char cfmt;
		
		/* use prepared format data */
		if (dlen) {
			if (hist->fmt.pos >= dlen) {
				fputs(endl, fd);
				hist->fmt.pos = 0;
			}
			cfmt = dat[hist->fmt.pos];
		}
		/* need current format information */
		else if (!(cfmt = hist->fmt.fmt)) {
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
		    && ((cfmt & MPT_ENUM(ByteOrderLittle)) != MPT_ENUM(ByteOrderNative))) {
			int i;
			for (i = 0; i < adv; ++i) buf[adv-i] = curr[i];
			curr = buf;
			cfmt = (cfmt & ~MPT_ENUM(ByteOrderLittle)) | MPT_ENUM(ByteOrderNative);
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
		/* stretch field size */
		if (conv < val.wdt) {
			conv = val.wdt;
		}
		/* field separation */
		if (hist->fmt.pos++) {
			fputc(' ', fd);
		}
		fwrite(buf, conv, 1, fd);
		
		done += adv;
		len -= adv;
		
		src = ((uint8_t *) src) + adv;
	}
	return done;
}
