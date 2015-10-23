/*!
 * set initial parameter for output descriptor
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "array.h"
#include "convert.h"

#include "output.h"

struct typeSource
{
	MPT_INTERFACE(source) ctl;
	const int8_t *src;
	size_t srclen, esze;
	const MPT_STRUCT(valfmt) *fmt;
	size_t fmtlen;
	char type;
};
static int getVoidType(MPT_INTERFACE(source) *src, int type, void *dest)
{
	struct typeSource *ts = (void *) src;
	ssize_t left;
	
	if (type == MPT_ENUM(TypeValFmt)) {
		if (!ts->fmtlen) {
			return -2;
		}
		if (dest) {
			*((MPT_STRUCT(valfmt) *) dest) = *ts->fmt;
		}
		++ts->fmt;
		--ts->fmtlen;
		return sizeof(*ts->fmt);
	}
	if (type != ts->type) return -1;
	if ((left = ts->srclen - (type = ts->esze)) < 0) return -2;
	if (dest) memcpy(dest, ts->src, type);
	ts->src += type;
	ts->srclen = left;
	return type;
}
static const MPT_INTERFACE_VPTR(source) getCtl = { getVoidType };

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
extern ssize_t mpt_history_print(FILE *fd, MPT_STRUCT(histinfo) *hist, size_t len, const void *src)
{
	struct typeSource ts;
	size_t total = 0;
	
	if (!fd) {
		return 0;
	}
	/* finish data block */
	if (!len) {
		fputc('\n', fd);
		return 1;
	}
	if (!src) {
		return -3;
	}
	/* not set up for run */
	if (!(ts.type = hist->type) || !(ts.esze = hist->size)) {
		return -4;
	}
	ts.ctl._vptr = &getCtl;
	ts.src = src;
	ts.srclen = len;
	
	while (len >= ts.esze) {
		MPT_STRUCT(buffer) *fmt;
		size_t pos = ts.srclen;
		
		if (!(fmt = hist->_fmt._buf)) {
			ts.fmtlen = 0;
		} else {
			ts.fmt = (void *) (fmt+1);
			ts.fmtlen = fmt->used/sizeof(*ts.fmt);
		}
		if (hist->line) {
			pos = hist->pos;
			
			/* skip data */
			if (pos >= hist->part) {
				size_t adv = hist->line - pos;
				
				/* line incomplete */
				if (adv > len) {
					hist->pos += adv = len;
				}
				/* finish line */
				else {
					hist->pos = 0;
					fputc('\n', fd);
				}
				/* advance positions */
				len -= adv;
				total += adv;
				ts.src += adv;
				continue;
			}
			ts.srclen = hist->part - pos;
			if (ts.srclen > len) {
				ts.srclen = len;
			}
			/* element not first in line */
			if (pos) {
				/* false `divide by 0` positive: ts.esze constant */
				if ((pos /= ts.esze) < ts.fmtlen) {
					ts.fmtlen -= pos;
					ts.fmt += pos;
				}
				fputc(' ', fd);
			}
			pos = ts.srclen;
		}
		else if (hist->pos) {
			fputc(' ', fd);
		}
		/* float values may have format information */
		if (mpt_fprint_val(fd, &ts.ctl) < 0) {
			return total;
		}
		pos -= ts.srclen;
		len -= pos;
		total += pos;
		hist->pos += pos;
	}
	return total;
}
