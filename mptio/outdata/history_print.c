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
	const char *prefix, *curr;
	int type, flags;
	
	if (hist->state & MPT_OUTFLAG(Active)) {
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
				if ((hist->mode & 0x80)
				    && (hist->mode & 0xf)) {
					static const char dots[] = "…";
					fputs(dots, fd);
				}
				if (hist->state & MPT_OUTFLAG(PrintRestore)) {
					fputs(mpt_ansi_reset(), fd);
					hist->state &= ~MPT_OUTFLAG(PrintRestore);
				}
				fputs(mpt_newline_string(hist->lsep), fd);
			}
			hist->state &= ~(0x7 | MPT_OUTFLAG(Active));
			return 0;
		}
		if (!(curr = src)) {
			return MPT_ERROR(MissingData);
		}
		if (!fd) {
			return len;
		}
		/* direct data write */
		if (!(hist->mode & 0x80)) {
			return fwrite(src, len, 1, fd);
		}
		/* detect delimiters */
		while (len--) {
			char val = *curr++;
			
			prefix = 0;
			switch (val) {
			  case 0x1: /* start of header */
				if ((hist->state & MPT_OUTFLAG(PrintColor))) {
					prefix = mpt_ansi_code(0);
				}
				val = hist->mode;
				/* previous segment and function */
				hist->mode = 0x80 | 0x40 | 0x20 | 0x1;
				break;
			  case 0x2:/* start of normal text */
				val = hist->mode;
				/* previous segment and text */
				hist->mode = 0x80 | 0x40 | 0x2;
				break;
			  case 0x3:/* end of normal text */
			  case 0x4:/* end of segment */
				val = hist->mode & 0x8f;
				/* previous segment */
				hist->mode = 0x80 | 0x40;
				break;
			  default:
				/* mark as processed */
				hist->mode |= 0x80 | 0x40;
				fputc(val, fd);
				continue;
			}
			/* end of function name */
			if (val & 0x20) {
				fputc('(', fd);
				fputc(')', fd);
			}
			/* segment end restore */
			if ((val & 0x80)
			    && (hist->state & MPT_OUTFLAG(PrintRestore))) {
				fputs(mpt_ansi_reset(), fd);
				hist->state &= ~MPT_OUTFLAG(PrintRestore);
			}
			/* has previous segment */
			if (val & 0x40) {
				fputc(':', fd);
				fputc(' ', fd);
			}
			if (prefix &&(isatty(fileno(fd)) > 0)) {
				fputs(prefix, fd);
				hist->state |= MPT_OUTFLAG(PrintRestore);
			}
		}
		return curr - (const char *) src;
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
		hist->mode = type = mt->arg;
		type &= 0x7f;
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
	/* discern log from regular output */
	if (!hist->file || fd == hist->file) {
		fputc('#', fd);
		fputc(' ', fd);
	}
	/* set prefix string */
	if ((hist->state & MPT_OUTFLAG(PrintColor))
	    && (prefix = mpt_ansi_code(type))
	    && (isatty(fileno(fd)) <= 0)) {
		prefix = 0;
	}
	/* mark answer message */
	if (mt->cmd == MPT_ENUM(MessageAnswer)) {
		if (prefix) {
			fputs(prefix, fd);
		}
		fputc('@', fd);
		if (mt->arg) {
			fprintf(fd, "[%d]: ", mt->arg);
		} else {
			fputc(':', fd);
			fputc(' ', fd);
		}
	}
	/* print (colorized) message type */
	else if ((curr = mpt_log_identifier(type))) {
		if (prefix) {
			fputs(prefix, fd);
		}
		fputc('[', fd);
		fputs(curr, fd);
		fputc(']', fd);
		fputc(' ', fd);
	} else {
		prefix = 0;
	}
	/* restore normal printing if needed */
	if (prefix && (prefix = mpt_ansi_reset())) {
		fputs(prefix, fd);
	}
	hist->state |= flags & 0x7;
	if (len > 2) {
		mpt_history_print(hist, len-2, mt+1);
	}
	return len;
}
