/*!
 * MPT core library
 *   configure log file data
 */

#define _POSIX_C_SOURCE 1 /* for fdopen() */

#include <stdio.h>
#include <string.h>
#include <strings.h> /* for strcasecmp() */

#include <unistd.h>

#include "meta.h"
#include "message.h"
#include "object.h"

#include "output.h"

static int setHistfile(MPT_STRUCT(logfile) *log, MPT_INTERFACE(convertable) *src)
{
	const char *where = 0;
	FILE *fd;
	int sock = -1;
	
	if (log->state & MPT_OUTFLAG(Active)) {
		return MPT_MESGERR(InProgress);
	}
	/* default output */
	if (!src) {
		fd = 0;
	}
	/* use socket descriptor */
	else if (src->_vptr->convert(src, MPT_ENUM(TypeSocket), &sock) >= 0) {
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
	else if (src->_vptr->convert(src, 's', &where) >= 0) {
		fd = 0;
		/* regular file path */
		if (where && !(fd = fopen(where, "w"))) {
			return MPT_ERROR(BadArgument);
		}
	}
	else {
		return MPT_ERROR(BadType);
	}
	if (log->file && (log->file != stdout) && (log->file != stderr)) {
		fclose(log->file);
	}
	log->file = fd;
	
	return 0;
}
/*!
 * \ingroup mptOutput
 * \brief set logfile parameters
 * 
 * Change parameter of supplied name for logfile.
 * 
 * \param hist  log file data
 * \param name  property to change
 * \param src   value source
 * 
 * \return consumed elements
 */
extern int mpt_logfile_set(MPT_STRUCT(logfile) *log, const char *name, MPT_INTERFACE(convertable) *src)
{
	int ret;
	if (!name) {
		return setHistfile(log, src);
	}
	if (!*name) {
		return MPT_ERROR(BadArgument);
	}
	if (!strcasecmp(name, "file")) {
		return setHistfile(log, src);
	}
	if (!strcasecmp(name, "ignore")) {
		uint8_t val;
		if (!src) {
			log->ignore = MPT_LOG(Info);
			return 0;
		}
		if ((ret = src->_vptr->convert(src, 'y', &val)) >= 0) {
			log->ignore = val;
		}
		return MPT_ERROR(BadValue);
	}
	if (!strcasecmp(name, "level")) {
		const char *ign = 0;
		if (!src) {
			log->ignore = MPT_LOG(Info);
			return 0;
		}
		if ((ret = src->_vptr->convert(src, 's', &ign)) >= 0) {
			int lv = mpt_log_level(ign);
			if (lv < 0) {
				return lv;
			}
			log->ignore = lv + 1;
		}
		return ret;
	}
	return MPT_ERROR(BadArgument);
}
/*!
 * \ingroup mptOutput
 * \brief get logfile parameters
 * 
 * Get data of supplied property for logfile.
 * 
 * \param log  log file data
 * \param pr   property to read
 * 
 * \return non-default length
 */
extern int mpt_logfile_get(const MPT_STRUCT(logfile) *log, MPT_STRUCT(property) *pr)
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
		pr->name = "logfile";
		pr->desc = MPT_tr("log message target");
		pr->val.fmt = fmt;
		pr->val.ptr = &log->file;
		return log->file ? 1 : 0;
	}
	id = 0;
	if (name ? (!strcasecmp(name, "file")) : pos == id++) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeFile), 0 };
		pr->name = "file";
		pr->desc = MPT_tr("output file descriptor");
		pr->val.fmt = fmt;
		pr->val.ptr = &log->file;
		return log->file ? 1 : 0;
	}
	if (name ? !strcasecmp(name, "ignore") : pos == id++) {
		pr->name = "ignore";
		pr->desc = MPT_tr("output message filter");
		pr->val.fmt = (uint8_t *) "y";
		pr->val.ptr = &log->ignore;
		return log->ignore != MPT_LOG(Info) ? 1 : 0;
	}
	return MPT_ERROR(BadArgument);
}
