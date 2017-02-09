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
extern ssize_t mpt_history_values(const MPT_STRUCT(histinfo) *hist, MPT_STRUCT(histfmt) *hfmt, size_t len, const void *src)
{
	MPT_STRUCT(valfmt) *fmt;
	MPT_STRUCT(buffer) *buf;
	FILE *fd;
	const char *endl, *dat;
	size_t flen, dlen;
	ssize_t done;
	
	if (!(fd = hist->file)) {
		if (len && !src) {
			return MPT_ERROR(BadOperation);
		}
		return len;
	}
	endl = mpt_newline_string(hist->lsep);
	
	/* finish data block */
	if (!len) {
		if (hfmt->fmt || hfmt->pos) {
			fputs(endl, fd);
			return 1;
		}
		return 0;
	}
	if (!src) {
		return MPT_ERROR(BadOperation);
	}
	done = 0;
	
	/* indicate setup state */
	if (!hfmt->fmt) {
		const int8_t *curr = src;
		
		if (*curr < 0) {
			return MPT_ERROR(BadValue);
		}
		/* separate type for data */
		if (!(hfmt->pos = *curr)) {
			hfmt->fmt = (int8_t) MPT_ENUM(ByteOrderLittle);
		}
		src = curr + 1;
		++done;
		--len;
	}
	dlen = 0;
	if ((buf = hfmt->_dat._buf)) {
		dat = (void *) (buf + 1);
		dlen = buf->used / sizeof(*dat);
	}
	/* require data format info */
	if (!(hfmt->fmt & 0x7f)) {
		while (dlen < hfmt->pos) {
			const int8_t *curr = src;
			
			if (!len--) {
				return done;
			}
			if (!(*curr & 0x7f)) {
				return MPT_ERROR(BadType);
			}
			if (!mpt_array_append(&hfmt->_dat, sizeof(*curr), curr)) {
				return MPT_ERROR(BadOperation);
			}
			++dlen;
			
			src = curr + 1;
		}
		/* update data type info */
		if ((buf = hfmt->_dat._buf)) {
			dat = (void *) (buf + 1);
			dlen = buf->used / sizeof(*dat);
		}
		hfmt->fmt = hfmt->pos;
		hfmt->pos = 0;
	}
	/* get otput format data */
	if ((buf = hfmt->_dat._buf)) {
		fmt = (void *) (buf + 1);
		flen = buf->used / sizeof(*fmt);
	}
	while (len) {
		char buf[256];
		MPT_STRUCT(valfmt) val;
		size_t adv;
		int curr;
		char cfmt;
		
		val.fmt = 0;
		val.wdt = 12;
		
		/* use prepared format data */
		if (dlen) {
			if (hfmt->pos >= dlen) {
				fputs(endl, fd);
				hfmt->pos = 0;
			}
			cfmt = dat[hfmt->pos];
		}
		/* need current format information */
		else if (!(cfmt = hfmt->all) && !(cfmt = hfmt->fmt)) {
			const int8_t *curr = src;
			
			if (!(cfmt = *curr)) {
				return done ? done : MPT_ERROR(BadType);
			}
			hfmt->fmt = cfmt;
			src = curr + 1;
			++done;
			
			if (!(--len)) {
				return done;
			}
			flen = 0;
			val.wdt = 0;
		}
		if (!(adv = mpt_msgvalfmt_size(cfmt)) < 0) {
			return MPT_ERROR(BadType);
		}
		if (len < adv) {
			return done;
		}
		/* determine output format */
		if (hfmt->pos < flen) {
			val = fmt[hfmt->pos];
		}
		else if (flen) {
			val = fmt[flen-1];
		}
		/* print number to buffer */
		if ((curr = mpt_number_print(buf, sizeof(buf), val, cfmt, src)) < 0) {
			return curr;
		}
		/* stretch field size */
		if (curr < val.wdt) {
			curr = val.wdt;
		}
		/* field separation */
		if (hfmt->pos) {
			fputc(' ', fd);
		}
		fwrite(buf, curr, 1, fd);
		
		done += adv;
		len -= adv;
		
		src = ((uint8_t *) src) + adv;
	}
	return done;
}
