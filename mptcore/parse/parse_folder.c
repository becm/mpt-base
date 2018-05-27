/*!
 * resolve alias to full library description
 */

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "config.h"
#include "output.h"

#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief read global mpt config
 * 
 * Process configuration file/directory.
 * 
 * \param cdir  configuration directory
 * \param log   optional log descriptor
 * \param save  path handler for configuration elements
 * \param ctx   handler context
 * 
 * \return solver creator library description
 */
extern int mpt_parse_folder(DIR *cfg, MPT_TYPE(PathHandler) save, void *ctx, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(parse) src = MPT_PARSE_INIT;
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_STRUCT(parsefmt) fmt = MPT_PARSEFMT_INIT;
	struct dirent *dent;
	int res = 0;
	int cdir;
	
	if (!save) {
		return MPT_ERROR(BadArgument);
	}
	if (!cfg) {
		return MPT_ERROR(MissingData);
	}
	if ((cdir = dirfd(cfg)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	src.src.getc = (int (*)(void *)) mpt_getchar_stdio;
	
	while ((dent = readdir(cfg))) {
		char buf[1024];
		int cfile;
		
		/* skip current/parent directory and invisible files */
		if (dent->d_name[0] == '.') {
			continue;
		}
		if ((cfile = openat(dirfd(cfg), dent->d_name, O_RDONLY)) < 0) {
			mpt_log(log, __func__, MPT_LOG(Warning), "%s: %s", MPT_tr("unable to read file"), dent->d_name);
			continue;
		}
		p.sep = 0;
		mpt_path_set(&p, buf, -1);
		p.len = p.off = 0;
		
		/* indicate new config file start */
		save(ctx, &p, 0, 0, 0);
		
		if (!(src.src.arg = fdopen(cfile, "r"))) {
			mpt_log(log, __func__, MPT_LOG(Error), "%s: %s", MPT_tr("unable to allocate file"), dent->d_name);
			close(cfile);
			continue;
		}
		src.src.line = 1;
		res = mpt_parse_config((MPT_TYPE(ParserFcn)) mpt_parse_format_pre, &fmt, &src, save, ctx);
		fclose(src.src.arg);
		if (log) {
			int line = src.src.line;
			if (res < 0) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %d (line %d): %s", MPT_tr("parse error"), res, line, dent->d_name);
			} else {
				mpt_log(log, __func__, MPT_LOG(Debug3), "%s: %s", MPT_tr("processed file"), dent->d_name);
			}
		}
		if (res < 0) {
			break;
		}
		res = 1;
	}
	return res;
}
