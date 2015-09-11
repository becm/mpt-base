/*!
 * set initial parameter for output descriptor
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "output.h"

struct typeSource
{
	MPT_INTERFACE(source) ctl;
	const int8_t *src;
	size_t len, esze;
	char type;
};
static int getVoidType(MPT_INTERFACE(source) *src, int type, void *dest)
{
	struct typeSource *ts = (void *) src;
	ssize_t left;
	if (type != ts->type) return -1;
	if ((left = ts->len - (type = ts->esze)) < 0) return -2;
	if (dest) memcpy(dest, ts->src, type);
	ts->src += type;
	ts->len  = left;
	return type;
}
static const MPT_INTERFACE_VPTR(source) getCtl = { getVoidType };

/*!
 * \ingroup mptMessage
 * \brief print data to file
 * 
 * Print available data elements to file.
 * 
 * \param fd	file descriptor
 * \param hist	history state information
 * \param len	length of data
 * \param src	start address of data
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
	ts.len = len;
	
	while (len >= ts.esze) {
		size_t pos = ts.len;
		
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
			else {
				ts.len = hist->part - pos;
				if (ts.len > len) {
					ts.len = len;
				}
				/* element not first in line */
				if (pos) fputc(' ', fd);
				pos = ts.len;
			}
		}
		
		/* float values may have format information */
		if (strchr("dfg", hist->type)) {
			const int16_t *fmt = hist->line ? hist->fmt : 0;
			size_t adv = hist->pos/ts.esze;
			while (fmt && adv--) {
				if (!*(fmt++)) fmt = 0;
			}
			mpt_fprint_float(fd, fmt, &ts.ctl);
		} else {
			mpt_fprint_int(fd, 0, &ts.ctl);
		}
		pos -= ts.len;
		len -= pos;
		total += pos;
		ts.src += pos;
		hist->pos += pos;
	}
	return total;
}
