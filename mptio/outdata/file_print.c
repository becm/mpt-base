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

#include "output.h"

static int answerType(int code)
{
	if (!code) {
		return MPT_LOG(Debug);
	}
	else if (code < 0) {
		return MPT_LOG(Error);
	}
	return MPT_LOG(Info);
}

/* print message */
static ssize_t outputWrite(FILE *fd, size_t len, const void *src)
{
	size_t all = 0;
	const uint8_t *sep;
	
	if (!len) {
		fputc('\n', fd);
		return 1;
	}
	while ((sep = memchr(src, 0, len))) {
		size_t diff = sep - ((const uint8_t *) src);
		if (diff) {
			fwrite(src, diff, 1, fd);
			all += diff;
		}
		fputc(':', fd);
		fputc(' ', fd);
		all += 2;
		len -= ++diff;
		src = ((const uint8_t *) src) + diff;
	}
	if (len && fwrite(src, len, 1, fd)) all += len;
	
	return all;
}

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
extern ssize_t mpt_file_print(uint8_t *state, FILE *hist, size_t len, const void *src)
{
	const MPT_STRUCT(msgtype) *mt;
	const char *prefix;
	int type, flags;
	
	if (*state & MPT_ENUM(OutputActive)) {
		if (!(*state & 0x7)) {
			return MPT_ERROR(BadArgument);
		}
		switch (*state & 0x3) {
		  case MPT_ENUM(OutputPrintNormal):
		    hist = stdout; break;
		  case MPT_ENUM(OutputPrintError):
		    hist = stderr; break;
		  case MPT_ENUM(OutputPrintHistory):
		    if (!hist) hist = stderr;
		    break;
		  default: hist = 0;
		}
		if (!len) {
			if (hist) {
				if ((*state & MPT_ENUM(OutputPrintRestore))
				    && isatty(fileno(hist))) {
					fputs(mpt_ansi_reset(), hist);
				}
				fputc('\n', hist);
			}
			*state &= ~(0x7 | MPT_ENUM(OutputActive));
			return 0;
		}
		if (!src) {
			return MPT_ERROR(MissingData);
		}
		if (hist) {
			outputWrite(hist, len, src);
		}
		return len;
	}
	if (!(mt = src)) {
		return MPT_ERROR(BadArgument);
	}
	/* limit min setup size */
	if (len < 2) {
		return MPT_ERROR(MissingData);
	}
	flags = *state & 0x7;
	*state &= ~0x7;
	
	/* setup answer output */
	if (mt->cmd == MPT_ENUM(MessageOutput)) {
		type = mt->arg;
	}
	else if (mt->cmd == MPT_ENUM(MessageAnswer)) {
		type = answerType(mt->arg);
	}
	else {
		return MPT_ERROR(BadType);
	}
	
	*state |= MPT_ENUM(OutputActive);
	switch (flags & 0x3) {
	  case 0:
		*state |= MPT_ENUM(OutputPrintRestore);
		return len;
	  case MPT_ENUM(OutputPrintNormal):
		hist = stdout;
		break;
	  case MPT_ENUM(OutputPrintError):
		hist = stderr;
		break;
	  case MPT_ENUM(OutputPrintHistory):
		if (hist) {
			flags = MPT_ENUM(OutputPrintHistory);
		} else {
			hist = stderr;
			flags = MPT_ENUM(OutputPrintError);
		}
		break;
	}
	if ((isatty(fileno(hist)) > 0) && (prefix = mpt_ansi_code(type))) {
		flags |= MPT_ENUM(OutputPrintRestore);
		fputs(prefix, hist);
	}
	/* mark answer message */
	if (mt->cmd == MPT_ENUM(MessageAnswer)) {
		fputc('@', hist);
		if (mt->arg) {
			fprintf(hist, "[%d]: ", mt->arg);
		} else {
			fputs(": ", hist);
		}
	}
	if (len > 2) {
		outputWrite(hist, len-2, mt+1);
	}
	*state |= flags & 0x7;
	
	return len;
}
