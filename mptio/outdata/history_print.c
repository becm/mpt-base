/*!
 * set initial parameter for output descriptor
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "array.h"
#include "convert.h"
#include "meta.h"

#include "output.h"

struct typeSource
{
	MPT_INTERFACE(metatype) ctl;
	const int8_t *src;
	size_t srclen, esze;
	const MPT_STRUCT(valfmt) *fmt;
	size_t fmtlen;
	char type;
};
static void histUnref(MPT_INTERFACE(metatype) *src)
{
	(void) src;
}
static int histAssign(MPT_INTERFACE(metatype) *src, const MPT_STRUCT(value) *val)
{
	(void) src; (void) val; return MPT_ERROR(BadOperation);
}
static int histConv(MPT_INTERFACE(metatype) *src, int type, void *dest)
{
	struct typeSource *ts = (void *) src;
	ssize_t left;
	size_t len;
	
	if ((type & 0xff) == MPT_ENUM(TypeValFmt)) {
		if (!ts->fmtlen) {
			return 0;
		}
		if (dest) {
			*((MPT_STRUCT(valfmt) *) dest) = *ts->fmt;
		}
		if (!(type & MPT_ENUM(ValueConsume))) {
			return MPT_ENUM(TypeValFmt);
		}
		++ts->fmt;
		--ts->fmtlen;
		return MPT_ENUM(TypeValFmt) | MPT_ENUM(ValueConsume);
	}
	if ((type & 0xff) != ts->type) {
		return MPT_ERROR(BadType);
	}
	len  = ts->esze;
	left = ts->srclen;
	
	if (!left) {
		return 0;
	}
	if ((left -= len) < 0) {
		return MPT_ERROR(BadOperation);
	}
	if (dest) memcpy(dest, ts->src, len);
	
	if (!(type & MPT_ENUM(ValueConsume))) {
		return ts->type | MPT_ENUM(ValueConsume);
	}
	ts->src += len;
	ts->srclen = left;
	return ts->type;
}
static MPT_INTERFACE(metatype) *histClone(MPT_INTERFACE(metatype) *src)
{
	(void) src; return 0;
}
static const MPT_INTERFACE_VPTR(metatype) getCtl = {
	histUnref,
	histAssign,
	histConv,
	histClone
};

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
		if (hist->pos) {
			fputc('\n', fd);
			return 1;
		}
		return 0;
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
