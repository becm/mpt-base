/*!
 * finalize connection data
 */

#define _POSIX_C_SOURCE 1 /* for fdopen() */

#include <stdio.h>
#include <string.h>
#include <strings.h> /* for strcasecmp() */

#include <unistd.h>

#include "meta.h"
#include "array.h"

#include "convert.h"

#include "output.h"

static int setHistfile(MPT_STRUCT(histinfo) *hist, const MPT_INTERFACE(metatype) *src)
{
	const char *where = 0;
	FILE *fd;
	int sock = -1;
	
	if (hist->state & MPT_OUTFLAG(Active)) {
		return MPT_ERROR(MessageInProgress);
	}
	/* default output */
	if (!src) {
		fd = stdout;
	}
	/* use socket descriptor */
	else if (src->_vptr->conv(src, MPT_ENUM(TypeSocket), &sock) >= 0) {
		if (sock < 0) {
			fd = 0;
		}
		else if ((sock = dup(sock)) < 0) {
			return MPT_ERROR(BadOperation);
		}
		else if (!(fd = fdopen(sock, "w"))) {
			close(sock);
			return MPT_ERROR(BadArgument);
		}
	}
	/* use file path */
	else if (src->_vptr->conv(src, 's', &where) >= 0) {
		fd = 0;
		/* regular file path */
		if (where && !(fd = fopen(where, "w"))) {
			return MPT_ERROR(BadArgument);
		}
	}
	else {
		return MPT_ERROR(BadType);
	}
	if (hist->file && (hist->file != stdout) && (hist->file != stderr)) {
		fclose(hist->file);
	}
	hist->file = fd;
	
	return 0;
}
/*!
 * \ingroup mptOutput
 * \brief set history parameters
 * 
 * Change history settings.
 * 
 * \param hist  history descriptor
 * \param name  property to change
 * \param src   value source
 * 
 * \return output descriptor
 */
extern int mpt_history_set(MPT_STRUCT(history) *hist, const char *name, const MPT_INTERFACE(metatype) *src)
{
	int ret;
	if (!name) {
		if (src) {
			return setHistfile(&hist->info, src);
		}
		mpt_history_fini(hist);
		hist->info.file = stdout;
		return 0;
	}
	if (!*name) {
		return MPT_ERROR(BadArgument);
	}
	if (!strcasecmp(name, "file")) {
		return setHistfile(&hist->info, src);
	}
	if (!strcasecmp(name, "format") || !strcasecmp(name, "fmt")) {
		return mpt_valfmt_set(&hist->fmt._fmt, src);
	}
	if (!strcasecmp(name, "ignore")) {
		uint8_t val;
		if (!src) {
			hist->info.ignore = MPT_LOG(Info);
			return 0;
		}
		if ((ret = src->_vptr->conv(src, 'y', &val)) >= 0) {
			hist->info.ignore = val;
		}
		return MPT_ERROR(BadValue);
	}
	if (!strcasecmp(name, "level")) {
		const char *ign = 0;
		if (!src) {
			hist->info.ignore = MPT_LOG(Info);
			return 0;
		}
		if ((ret = src->_vptr->conv(src, 's', &ign)) >= 0) {
			int lv = mpt_log_level(ign);
			if (lv < 0) {
				return lv;
			}
			hist->info.ignore = lv + 1;
		}
		return ret;
	}
	return MPT_ERROR(BadArgument);
}
/*!
 * \ingroup mptOutput
 * \brief get history parameters
 * 
 * Read history settings.
 * 
 * \param hist  history descriptor
 * \param pr    property to read
 * 
 * \return output descriptor
 */
extern int mpt_history_get(const MPT_STRUCT(history) *hist, MPT_STRUCT(property) *pr)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!pr) {
		return MPT_ENUM(TypeFile);
	}
	if (!(name = pr->name)) {
		pos = (intptr_t) pr->desc;
	}
	else if (!*name) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeFile), 0 };
		pr->name = "history";
		pr->desc = MPT_tr("output values and messages");
		pr->val.fmt = fmt;
		pr->val.ptr = &hist->info.file;
		return hist->info.file ? 1 : 0;
	}
	id = 0;
	if (name ? (!strcasecmp(name, "file") || !strcasecmp(name, "history") || !strcasecmp(name, "histfile")) : pos == id++) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeFile), 0 };
		pr->name = "file";
		pr->desc = MPT_tr("history data output file");
		pr->val.fmt = fmt;
		pr->val.ptr = &hist->info.file;
		return hist->info.file ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "format") || !strcasecmp(name, "histfmt") || !strcasecmp(name, "fmt")) :  pos == id++) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeValFmt), 0 };
		MPT_STRUCT(buffer) *buf;
		int len;
		pr->name = "format";
		pr->desc = MPT_tr("history data output format");
		pr->val.fmt = fmt;
		pr->val.ptr = 0;
		if (!(buf = hist->fmt._fmt._buf)
		    || !(len = buf->_used / sizeof(MPT_STRUCT(valfmt)))) {
			return 0;
		}
		pr->val.ptr = buf + 1;
		return len;
	}
	if (name ? !strcasecmp(name, "ignore") : pos == id++) {
		pr->name = "ignore";
		pr->desc = MPT_tr("output message filter");
		pr->val.fmt = (uint8_t *) "y";
		pr->val.ptr = &hist->info.ignore;
		return hist->info.ignore != MPT_LOG(Info) ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
