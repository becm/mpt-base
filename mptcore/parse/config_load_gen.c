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

#include "parse.h"

static int acceptAll(void *ctx, const MPT_STRUCT(path) *p, int last, int curr)
{
	(void) ctx;
	(void) p;
	(void) last;
	(void) curr;
	return 0;
}

int loadFile(int cfile, MPT_STRUCT(parse) *src, MPT_TYPE(PathHandler) save, void *ctx)
{
	MPT_STRUCT(parsefmt) fmt = MPT_PARSEFMT_INIT;
	int res;
	
	if (!(src->src.arg = fdopen(cfile, "r"))) {
		return 0;
	}
	src->src.line = 1;
	if (!save) save = acceptAll;
	res = mpt_parse_config((MPT_TYPE(ParserFcn)) mpt_parse_format_pre, &fmt, src, save, ctx);
	fclose(src->src.arg);
	save(ctx, 0, 0, 0);
	return res;
}

int loadDir(int cdir, const char *name, MPT_TYPE(PathHandler) save, void *ctx, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(parse) src = MPT_PARSE_INIT;
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	DIR *cfg;
	struct dirent *dent;
	
	if (!(cfg = fdopendir(cdir))) {
		return 0;
	}
	src.src.getc = (int (*)(void *))  mpt_getchar_stdio;
	
	while ((dent = readdir(cfg))) {
		static const char _func[] = "mpt_config_load\0";
		static const char dir[] = "/etc/mpt.conf.d\0";
		
		char buf[1024];
		int res, cfile;
		
		
		if (dent->d_name[0] == '.') {
			continue;
		}
		if ((cfile = openat(cdir, dent->d_name, O_RDONLY)) < 0) {
			continue;
		}
		if (name) {
			snprintf(buf, sizeof(buf), "%s/%s/%s", name, dir+1, dent->d_name);
		} else {
			snprintf(buf, sizeof(buf), "%s/%s", dir, dent->d_name);
		}
		p.sep = 0;
		mpt_path_set(&p, buf, -1);
		p.len = p.off = 0;
		if (save) save(ctx, &p, 0, 0);
		
		res = loadFile(cfile, &src, save, ctx);
		close(cfile);
		
		if (log) {
			int line = src.src.line;
			if (res < 0) {
				mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %d (line %d): %s", MPT_tr("parse error"), res, line, buf);
			} else {
				mpt_log(log, _func, MPT_FCNLOG(Debug3), "%s: %s", MPT_tr("processed file"), buf);
			}
		}
		if (res < 0) {
			closedir(cfg);
			return res;
		}
	}
	closedir(cfg);
	return 1;
}
/*!
 * \ingroup mptParse
 * \brief read global mpt config
 * 
 * Process configuration file/directory.
 * 
 * \param root  configuration in alternative file root
 * \param log   optional log descriptor
 * \param save  path handler for configuration elements
 * \param ctx   handler context
 * 
 * \return solver creator library description
 */
extern int _mpt_config_load(const char *root, MPT_INTERFACE(logger) *log, MPT_TYPE(PathHandler) save, void *ctx)
{
	static const char sol_cdir[] = "/etc/mpt.conf.d\0";
	static const char sol_cfile[] = "/etc/mpt.conf\0";
	
	int cfg, ret = 0;
	
	/* try prefixed config directory */
	if (root) {
		int sub;
		if ((sub = open(root, O_RDONLY | O_DIRECTORY)) < 0) {
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s: %s", MPT_tr("unable to open alternative root"), root);
			return MPT_ERROR(BadArgument);
		}
		if ((cfg = openat(sub, sol_cfile+1, O_RDONLY)) >= 0) {
			MPT_STRUCT(parse) src = MPT_PARSE_INIT;
			MPT_STRUCT(path) p = MPT_PATH_INIT;
			char buf[1024];
			int curr;
			
			snprintf(buf, sizeof(buf), "%s%s", root, sol_cdir);
			
			p.sep = 0;
			mpt_path_set(&p, buf, -1);
			p.len = p.off = 0;
			if (save) save(ctx, &p, 0, 0);
			
			src.src.getc = (int (*)(void *))  mpt_getchar_stdio;
			curr = loadFile(cfg, &src, save, ctx);
			close(cfg);
			
			if (log) {
				if (curr < 0) {
					mpt_log(log, __func__, MPT_FCNLOG(Error), "%s [%d]: %s", MPT_tr("parse error"), curr, buf);
					return curr;
				} else {
					mpt_log(log, __func__, MPT_FCNLOG(Debug), "%s: %s", MPT_tr("processed file"), buf);
				}
			}
			else if (curr < 0) {
				return curr;
			}
			ret |= 2;
		}
		if ((cfg = openat(sub, sol_cdir+1, O_RDONLY | O_DIRECTORY)) >= 0) {
			int curr;
			curr = loadDir(cfg, root, save, ctx, log);
			close(cfg);
			
			if (curr < 0) {
				return curr;
			}
			ret |= 1;
		}
		close(sub);
		
		return ret;
	}
	/* global config dir */
	if ((cfg = open(sol_cfile, O_RDONLY)) >= 0) {
		MPT_STRUCT(parse) src = MPT_PARSE_INIT;
		int curr;
		curr = loadFile(cfg, &src, save, ctx);
		close(cfg);
		if (curr < 0) {
			return curr;
		}
		if (log) {
			if (curr < 0) {
				mpt_log(log, __func__, MPT_FCNLOG(Error), "%s [%d]: %s", MPT_tr("parse error"), curr, sol_cfile);
				return curr;
			} else {
				mpt_log(log, __func__, MPT_FCNLOG(Debug), "%s: %s", MPT_tr("processed file"), sol_cfile);
			}
		}
		else if (curr < 0) {
			return curr;
		}
		ret |= 1;
	}
	if ((cfg = open(sol_cdir, O_RDONLY | O_DIRECTORY)) >= 0) {
		int curr;
		curr = loadDir(cfg, 0, save, ctx, log);
		close(cfg);
		if (curr < 0) {
			return curr;
		}
		ret |= 2;
	}
	return ret;
}

