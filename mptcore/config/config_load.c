/*!
 * MPT core library
 *   load configuration
 */

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include "meta.h"
#include "config.h"
#include "output.h"
#include "types.h"

#include "parse.h"

struct loadCtx
{
	MPT_INTERFACE(config) *cfg;
	MPT_INTERFACE(logger) *log;
	const char *fcn;
};

static int cfgSet(void *ptr, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val, int last, int curr)
{
	struct loadCtx *ctx = ptr;
	MPT_INTERFACE(config) *cfg;
	
	if ((curr & 0x3) != MPT_PARSEFLAG(Option)) {
		return 0;
	}
	cfg = ctx->cfg;
	if ((last = cfg->_vptr->assign(cfg, p, val)) < 0) {
		if (ctx->log) {
			mpt_log(ctx->log, ctx->fcn, MPT_LOG(Error), "%s",
			        MPT_tr("failed to set global config element"));
		}
		return last;
	}
	return 0;
}
/*!
 * \ingroup mptConfig
 * \brief read global mpt config
 * 
 * Process configuration file/directory.
 * 
 * \param root  configuration in alternative file root
 * \param log   optional log descriptor
 * \param dest  target config path
 * 
 * \return configuration load result
 */
extern int mpt_config_load(MPT_INTERFACE(config) *cfg, const char *root, MPT_INTERFACE(logger) *log)
{
	static const char _etc[]  = "/etc";
	static const char _file[] = "mpt.conf";
	static const char _dir[]  = "mpt.conf.d";
	
	MPT_INTERFACE(metatype) *mt;
	const char *subdir = "";
	struct loadCtx ctx;
	int dir, fd, ret;
	
	if (!root) {
		const char *base;
		
		/* detect root location and mode */
		base = getenv("MPT_PREFIX");
		root = base ? base : _etc;
		
		/* top level config location may not exist */
		if ((dir = open(root, O_RDONLY | O_DIRECTORY)) < 0) {
			mpt_log(log, __func__, MPT_LOG(Warning), "%s: %s",
			        MPT_tr("failed to open config root"), root);
			return 0;
		}
		/* switch to subdir in generic MPT prefix mode */
		if (base) {
			int sub = openat(dir, _etc + 1, O_RDONLY | O_DIRECTORY);
			
			close(dir);
			/* failure in generic mode is not an error */
			if (sub < 0) {
				mpt_log(log, __func__, MPT_LOG(Info), "%s: %s%s",
				        MPT_tr("failed to open config root"), base, _etc);
				return 0;
			}
			subdir = _etc;
			dir = sub;
		}
	}
	else if ((dir = open(root, O_RDONLY | O_DIRECTORY)) < 0) {
		mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
		        MPT_tr("failed to open config root"), root);
		return MPT_ERROR(MissingData);
	}
	mt = 0;
	if (!cfg) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		mpt_path_set(&p, "mpt", -1);
		if (!(mt = mpt_config_global(&p))) {
			mpt_log(log, __func__, MPT_LOG(Fatal), "%s",
			        MPT_tr("failed to access mpt config"));
			return MPT_ERROR(BadOperation);
		}
		if ((ret = MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &cfg)) < 0
		    || !cfg) {
			mpt_log(log, __func__, MPT_LOG(Fatal), "%s",
			        MPT_tr("bad global config element"));
			mt->_vptr->unref(mt);
			return MPT_ERROR(BadType);
		}
	}
	ctx.cfg = cfg;
	ctx.fcn = __func__;
	ctx.log = log;
	ret = 0;
	if ((fd = openat(dir, _dir, O_RDONLY | O_DIRECTORY)) >= 0) {
		DIR *dirp;
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Debug3), "%s: %s%s/%s",
			        MPT_tr("process config directory"), root, subdir, _dir);
		}
		if (!(dirp = fdopendir(fd))) {
			close(fd);
			ret |= 2;
		}
		else {
			if (mpt_parse_folder(dirp, cfgSet, &ctx, log) >= 0) {
				ret |= 2;
			}
			closedir(dirp);
		}
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Debug3), "%s: %s%s/%s",
			        MPT_tr("finished config directory"), root, subdir, _dir);
		}
	}
	if ((fd = openat(dir, _file, O_RDONLY)) >= 0) {
		MPT_STRUCT(parser_context) src = MPT_PARSER_INIT;
		MPT_STRUCT(parser_format) fmt = MPT_PARSER_FORMAT_INIT;
		int err;
		if (!(src.src.arg = fdopen(fd, "r"))) {
			if (mt) {
				mt->_vptr->unref(mt);
			}
			return 0;
		}
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Debug3), "%s: %s%s/%s",
			        MPT_tr("process config file"), root, subdir, _file);
		}
		/* allow default format override */
		mpt_parse_format(&fmt, 0);
		src.src.getc = (int (*)(void *))  mpt_getchar_stdio;
		src.src.line = 1;
		err = mpt_parse_config((MPT_TYPE(input_parser)) mpt_parse_format_pre, &fmt, &src, cfgSet, &ctx);
		if (err >= 0) ret |= 1;
		fclose(src.src.arg);
		
		if (err < 0) {
			int line = src.src.line;
			mpt_log(log, __func__, MPT_LOG(Info), "%s [%d] (line = %d): %s%s/%s",
			        MPT_tr("config file error"), err, line, root, subdir, _file);
		}
		else if (log) {
			mpt_log(log, __func__, MPT_LOG(Debug3), "%s: %s%s/%s",
			        MPT_tr("finished config file"), root, subdir, _file);
		}
	}
	if (mt) {
		mt->_vptr->unref(mt);
	}
	close(dir);
	return ret;
}
