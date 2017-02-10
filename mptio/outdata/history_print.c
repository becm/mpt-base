/*!
 * MPT core library
 *   printable output setup and processing
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "array.h"
#include "message.h"
#include "convert.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief write text message parts
 * 
 * Check for type on initial data and set
 * state for continous call.
 * 
 * \param state  output state flags
 * \param hist   history file
 * \param len    size of new data
 * \param src    data to add
 * 
 * \return output file descriptor
 */
extern ssize_t mpt_history_print(MPT_STRUCT(histinfo) *hist, size_t len, const void *src)
{
	const MPT_STRUCT(msgtype) *mt;
	FILE *fd;
	const char *prefix;
	int type, flags;
	
	if (hist->state & MPT_OUTFLAG(Active)) {
		const char *pos, *end;
		size_t part;
		if (!(hist->state & 0x7)) {
			return MPT_ERROR(BadArgument);
		}
		switch (hist->state & 0x3) {
		  case MPT_OUTFLAG(PrintNormal):
		    fd = stdout; break;
		  case MPT_OUTFLAG(PrintError):
		    fd = stderr; break;
		  case MPT_OUTFLAG(PrintHistory):
		    if (!(fd = hist->file)) fd = stderr;
		    break;
		  default: fd = 0;
		}
		if (!len) {
			if (fd) {
				/* incomplete message */
				if (hist->mode) {
					static const char dots[] = "…";
					fputs(dots, fd);
				}
				if ((hist->state & MPT_OUTFLAG(PrintRestore))
				    && isatty(fileno(fd))) {
					fputs(mpt_ansi_reset(), fd);
					hist->state &= ~MPT_OUTFLAG(PrintRestore);
				}
				fputs(mpt_newline_string(hist->lsep), fd);
			}
			hist->state &= ~(0x7 | MPT_OUTFLAG(Active));
			return 0;
		}
		if (!(pos = src)) {
			return MPT_ERROR(MissingData);
		}
		if (!fd) {
			return len;
		}
		end = pos + len;
		part = 0;
		while ((pos + part) < end) {
			char curr = pos[part];
			/* detect delimiters */
			if (curr == 0x1) {
				if (part) fwrite(pos, part, 1, fd);
				pos += part + 1;
				part = 0;
				hist->mode = curr;
				continue;
			}
			if (curr == 0x2) {
				if (part) fwrite(pos, part, 1, fd);
				pos += part + 1;
				part = 0;
				/* end of function name */
				if (hist->mode == 0x1) {
					fputc('(', fd);
					fputc(')', fd);
				}
				if (hist->mode) {
					fputc(':', fd);
					fputc(' ', fd);
				}
				if ((hist->state & MPT_OUTFLAG(PrintRestore))
				    && isatty(fileno(fd))) {
					fputs(mpt_ansi_reset(), fd);
					hist->state &= ~MPT_OUTFLAG(PrintRestore);
				}
				hist->mode = curr;
				continue;
			}
			if (curr == 0x3) {
				if (part) fwrite(pos, part, 1, fd);
				pos += part + 1;
				part = 0;
				hist->mode = 0;
				continue;
			}
			++part;
		}
		if (part) fwrite(pos, part, 1, fd);
		return pos - (char *) src;
	}
	if (!(mt = src)) {
		return MPT_ERROR(BadArgument);
	}
	/* limit min setup size */
	if (len < 2) {
		return MPT_ERROR(MissingData);
	}
	hist->state &= ~0x7;
	hist->mode = 0;
	
	if (mt->cmd == MPT_ENUM(MessageOutput)) {
		type = mt->arg;
		hist->mode = ' ';
	}
	/* setup answer output */
	else if (mt->cmd == MPT_ENUM(MessageAnswer)) {
		if (mt->arg < 0) {
			type = MPT_LOG(Error);
		} else {
			type = mt->arg ? MPT_LOG(Info) : MPT_LOG(Debug);
		}
	}
	else {
		return MPT_ERROR(BadType);
	}
	flags = mpt_output_type(type, hist->ignore);
	
	hist->state |= MPT_OUTFLAG(Active);
	switch (flags & 0x3) {
	  case 0:
		hist->state |= MPT_OUTFLAG(PrintRestore);
		return len;
	  case MPT_OUTFLAG(PrintNormal):
		fd = stdout;
		break;
	  case MPT_OUTFLAG(PrintError):
		fd = stderr;
		break;
	  case MPT_OUTFLAG(PrintHistory):
		if ((fd = hist->file)) {
			flags = MPT_OUTFLAG(PrintHistory);
		} else {
			flags = MPT_OUTFLAG(PrintError);
			fd = stderr;
		}
		break;
	}
	if ((isatty(fileno(fd)) > 0)
	    && (hist->state & MPT_OUTFLAG(PrintColor))
	    && (prefix = mpt_ansi_code(type))) {
		flags |= MPT_OUTFLAG(PrintRestore);
		fputs(prefix, fd);
	}
	/* mark answer message */
	if (mt->cmd == MPT_ENUM(MessageAnswer)) {
		fputc('@', fd);
		if (mt->arg) {
			fprintf(fd, "[%d]: ", mt->arg);
		} else {
			fputc(':', fd);
			fputc(' ', fd);
		}
	}
	else if (!prefix && (prefix = mpt_log_identifier(type))) {
		fputc('[', fd);
		fputs(prefix, fd);
		fputc(']', fd);
		fputc(' ', fd);
	}
	hist->state |= flags & 0x7;
	if (len > 2) {
		mpt_history_print(hist, len-2, mt+1);
	}
	return len;
}
