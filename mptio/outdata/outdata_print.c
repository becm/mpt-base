/*!
 * MPT core library
 *   printable output setup and processing
 */

#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "array.h"
#include "message.h"

#include "output.h"

#define AnswerFlags(a) ((a & 0xf0) >> 4)
#define OutputFlags(a) (a & 0xf)

static int answerType(int code)
{
	if (!code) {
		return MPT_ENUM(LogDebug);
	}
	else if (code < 0) {
		return MPT_ENUM(LogError);
	}
	return MPT_ENUM(LogInfo);
}
/* print message */
static ssize_t outputWrite(FILE *fd, size_t len, const void *src)
{
	size_t all = 0;
	const uint8_t *sep;
	
	if (!(fd)) return 0;
	
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
 * \brief print message processing
 * 
 * Check for type on initial data and set outdata
 * state for continous call.
 * 
 * \param od   output data
 * \param len  size of new data
 * \param src  data to add
 * 
 * \return output file descriptor
 */
extern int mpt_outdata_print(MPT_STRUCT(outdata) *od, FILE *hist, size_t len, const void *src)
{
	const MPT_STRUCT(msgtype) *mt;
	const char *prefix;
	int type, flags;
	
	if (od->state & MPT_ENUM(OutputActive)) {
		if (!(od->state & 0x7)) {
			return -1;
		}
		switch (od->state & 0x3) {
		  case MPT_ENUM(OutputPrintNormal):
		    hist = stdout; break;
		  case MPT_ENUM(OutputPrintError):
		    hist = stderr; break;
		  case MPT_ENUM(OutputPrintHistory):
		    if (!hist) hist = stderr;
		    break;
		  default: return 0;
		}
		if (!len) {
			if ((od->state & MPT_ENUM(OutputPrintRestore))
			    && isatty(fileno(hist))) {
				fputs("\033[0m", hist);
			}
			fputc('\n', hist);
			od->state &= ~(0x7 | MPT_ENUM(OutputActive));
			return 0;
		}
		if (!src) {
			return -2;
		}
		outputWrite(hist, len, src);
		
		return len;
	}
	/* limit min setup size */
	if (len < 2 || !(mt = src)) {
		return -2;
	}
	od->state &= ~(0x7 | MPT_ENUM(OutputActive));
	
	/* setup answer output */
	if (mt->cmd == MPT_ENUM(MessageOutput)) {
		type  = mt->arg;
		flags = OutputFlags(od->level);
	}
	else if (mt->cmd == MPT_ENUM(MessageAnswer)) {
		type  = answerType(mt->arg);
		flags = AnswerFlags(od->level);
	}
	else {
		return -1;
	}
	flags = mpt_output_file(type, flags);
	
	switch (flags & 0x3) {
	  case 0:
		hist = 0;
		break;
	  case MPT_ENUM(OutputPrintNormal):
		hist = stdout;
		break;
	  case MPT_ENUM(OutputPrintError):
		hist = stderr;
		break;
	  case MPT_ENUM(OutputPrintHistory):
		break;
	}
	if (flags & MPT_ENUM(OutputPrintRestore)) {
		if (hist) {
			flags = MPT_ENUM(OutputPrintHistory);
		} else {
			hist = stderr;
			flags = MPT_ENUM(OutputPrintError);
		}
	}
	od->state |= MPT_ENUM(OutputActive);
	
	if (!hist) {
		od->state |= MPT_ENUM(OutputPrintRestore);
		return 0;
	}
	if ((isatty(fileno(hist)) > 0) && (prefix = mpt_output_prefix(type))) {
		flags |= MPT_ENUM(OutputPrintRestore);
		fputs(prefix, hist);
	}
	/* mark answer message */
	if (mt->cmd == MPT_ENUM(MessageAnswer)) {
		fputc('@', hist);
	}
	if (len > 2) {
		outputWrite(hist, len-2, mt+1);
	}
	od->state |= flags & 0x7;
	
	return len;
}
