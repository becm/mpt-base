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

struct typeSource
{
	MPT_INTERFACE(metatype) ctl;
	FILE *fd;
	struct {
		const uint8_t *ptr;
		size_t len;
	} src;
	struct {
		const MPT_STRUCT(valfmt) *ptr;
		size_t len, pos;
	} fmt;
	struct {
		const MPT_STRUCT(msgvalfmt) *ptr;
		size_t len, pos;
	} part;
	size_t size;
	struct {
		uint8_t pos, max;
	} elem;
	char type;
};
static int advancePart(struct typeSource *ts)
{
	uint8_t pos;
	
	if (!ts->part.ptr) {
		return ts->type;
	}
	ts->elem.pos = 0;
	while ((pos = ++ts->part.pos) < ts->part.len) {
		int type, fmt;
		/* use part elements */
		if (!(ts->elem.max = ts->part.ptr[pos].len)) {
			continue;
		}
		fmt = ts->part.ptr[pos].fmt;
		if ((type = mpt_msgvalfmt_type(fmt)) < 0) {
			return type;
		}
		ts->type = type;
		ts->size = mpt_msgvalfmt_size(fmt);
		
		return type;
	}
	/* all parts processed */
	fputc('\n', ts->fd);
	ts->fmt.pos = 0;
	ts->part.pos = 0;
	ts->elem.max = 0;
	return 0;
}
static void histUnref(MPT_INTERFACE(unrefable) *src)
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
	
	left = ts->src.len;
	
	/* advance to next segment */
	if (ts->elem.pos == ts->elem.max) {
		int type;
		
		if ((type = advancePart(ts)) < 0) {
			return type;
		}
		if (!type) {
			return 0;
		}
	}
	if (!left) {
		return 0;
	}
	len = ts->size;
	if ((left -= len) < 0) {
		return MPT_ERROR(MissingData);
	}
	if ((type & 0xff) == MPT_ENUM(TypeValFmt)) {
		if (!ts->fmt.ptr) {
			return MPT_ERROR(BadValue);
		}
		if (dest) {
			*((MPT_STRUCT(valfmt) *) dest) = ts->fmt.ptr[ts->fmt.pos];
		}
		/* no advance on last element */
		if (!(type & MPT_ENUM(ValueConsume)) || !ts->fmt.len) {
			return MPT_ENUM(TypeValFmt);
		}
		if (++ts->fmt.pos < ts->fmt.len) {
			return MPT_ENUM(TypeValFmt) | MPT_ENUM(ValueConsume);
		}
		--ts->fmt.pos;
		return MPT_ENUM(TypeValFmt);
	}
	if ((type & 0xff) != ts->type) {
		return MPT_ERROR(BadType);
	}
	if (dest) memcpy(dest, ts->src.ptr, len);
	
	if (ts->part.pos || ts->elem.pos) {
		fputc(' ', ts->fd);
	}
	if (!(type & MPT_ENUM(ValueConsume))) {
		return ts->type;
	}
	++ts->elem.pos;
	ts->src.ptr += len;
	ts->src.len  = left;
	
	return ts->type | MPT_ENUM(ValueConsume);
}
static MPT_INTERFACE(metatype) *histClone(const MPT_INTERFACE(metatype) *src)
{
	(void) src; return 0;
}
static const MPT_INTERFACE_VPTR(metatype) getCtl = {
	{ histUnref },
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
	MPT_STRUCT(buffer) *buf;
	struct typeSource ts;
	int err;
	
	if (!fd) {
		return 0;
	}
	/* finish data block */
	if (!(ts.src.len = len)) {
		if (hist->pos.fmt || hist->pos.elem) {
			fputc('\n', fd);
			return 1;
		}
		return 0;
	}
	if (!(ts.src.ptr = src)) {
		return MPT_ERROR(BadOperation);
	}
	/* indicate setup state */
	if (hist->lfmt & 0xf) {
		MPT_STRUCT(msgvalfmt) *fmt;
		/* invalid header format type */
		if (hist->lfmt & MPT_ENUM(ValuesBig)
		    && !hist->pos.fmt) {
			MPT_STRUCT(buffer) *buf;
			fmt = 0;
			if ((buf = hist->_dat._buf) && buf->used) {
				fmt = ((MPT_STRUCT(msgvalfmt) *)(buf+1)) + (buf->used/sizeof(*fmt)) - 1;
			}
			/* alternative format */
			while (1) {
				MPT_STRUCT(msgvalfmt) tmp;
				/* setup finished */
				if (!(tmp.fmt = *ts.src.ptr)) {
					/* fallback to single value type */
					if (!fmt) {
						hist->pos.fmt = hist->lfmt;
					}
					hist->lfmt &= 0xf0;
					if (!--ts.src.len) {
						return len;
					}
					break;
				}
				tmp.len = 1;
				if (fmt && (tmp.fmt == fmt->fmt) && fmt->len < UINT8_MAX) {
					++fmt->len;
				}
				else if ((tmp.fmt & 0x7f) && !(fmt = mpt_array_append(&hist->_dat, sizeof(tmp), &tmp))) {
					return MPT_ERROR(BadOperation);
				}
				if (!++hist->fpos) {
					return MPT_ERROR(MissingBuffer);
				}
				if (!--ts.src.len) {
					return len;
				}
				fmt = (void *) ++ts.src.ptr;
			}
		}
		/* need format information */
		else {
			hist->lfmt &= ~MPT_ENUM(ValuesBig);
			while (hist->pos.fmt) {
				fmt = (void *) ts.src.ptr;
				if (ts.src.len < sizeof(*fmt)) {
					ssize_t total = len - ts.src.len;
					return total ? total : MPT_ERROR(MissingData);
				}
				if (!mpt_array_append(&hist->_dat, sizeof(*fmt), fmt)) {
					return MPT_ERROR(BadOperation);
				}
				--hist->pos.fmt;
				if (!++hist->fpos) {
					return MPT_ERROR(MissingBuffer);
				}
				ts.src.ptr  = (void *) ++fmt;
				ts.src.len -= sizeof(*fmt);
			}
			if (!(buf = hist->_dat._buf) || !buf->used) {
				hist->pos.fmt = hist->lfmt;
			}
			hist->lfmt &= 0xf0;
		}
		hist->fpos = 0;
	}
	
	ts.ctl._vptr = &getCtl;
	ts.fd = fd;
	
	if (!(buf = hist->_fmt._buf)
	    || !(ts.fmt.len = buf->used / sizeof(*ts.fmt.ptr))
	    || (hist->lfmt & MPT_ENUM(ValuesBig))) {
		ts.fmt.ptr = 0;
	} else {
		ts.fmt.ptr = (void *) (buf+1);
		ts.fmt.pos = hist->fpos;
		if (ts.fmt.pos >= ts.fmt.len) {
			return MPT_ERROR(BadArgument);
		}
	}
	/* unified non-segmented data */
	if (!(buf = hist->_dat._buf)
	    || !(ts.size = buf->used / sizeof(*ts.part.ptr))) {
		int type;
		ts.part.ptr = 0;
		ts.part.len = 0;
		ts.part.pos = 0;
		
		if ((type = mpt_msgvalfmt_type(hist->pos.fmt)) < 0) {
			return type;
		}
		ts.type = type;
		ts.size = mpt_msgvalfmt_size(hist->pos.fmt);
	}
	/* data part information */
	else {
		int type;
		uint8_t fmt;
		ts.part.ptr = (void *) (buf+1);
		ts.part.len = ts.size;
		ts.part.pos = hist->pos.fmt;
		if (ts.part.pos >= ts.part.len) {
			return MPT_ERROR(BadArgument);
		}
		ts.elem.max = ts.part.ptr[ts.part.pos].len;
		ts.elem.pos = hist->pos.elem;
		if (ts.elem.pos > ts.elem.max) {
			return MPT_ERROR(BadArgument);
		}
		fmt = ts.part.ptr[ts.part.pos].fmt;
		if ((type = mpt_msgvalfmt_type(fmt)) < 0) {
			return type;
		}
		ts.type = type;
		ts.size = mpt_msgvalfmt_size(fmt);
	}
	
	while (1) {
		ssize_t total;
		err = mpt_fprint_val(fd, &ts.ctl);
		
		hist->pos.fmt  = ts.part.pos;
		hist->pos.elem = ts.elem.pos;
		hist->fpos = ts.fmt.pos;
		
		total = len - ts.src.len;
		if (err < 0) {
			return total ? total : err;
		}
		if (!ts.src.len) {
			return total;
		}
		
		hist->pos.fmt = 0;
		hist->pos.elem = 0;
		hist->fpos = 0;
	}
}
