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
	static const char _file[] = "etc/mpt.conf";
	static const char _dir[]  = "etc/mpt.conf.d";
	
	MPT_INTERFACE(metatype) *mt;
	struct loadCtx ctx;
	int dir, fd, ret;
	
	if (!root && !(root = getenv("MPT_PREFIX"))) {
		root = "/";
	}
	if ((dir = open(root, O_RDONLY | O_DIRECTORY)) < 0) {
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
		if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg)) < 0
		    || !cfg) {
			mpt_log(log, __func__, MPT_LOG(Fatal), "%s",
			        MPT_tr("bad global config element"));
			mt->_vptr->ref.unref((void *) mt);
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
			mpt_log(log, __func__, MPT_LOG(Debug3), "%s: %s/%s",
			        MPT_tr("process config directory"), root, _dir);
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
			mpt_log(log, __func__, MPT_LOG(Debug3), "%s: %s/%s",
			        MPT_tr("finished config directory"), root, _dir);
		}
	}
	if ((fd = openat(dir, _file, O_RDONLY)) >= 0) {
		MPT_STRUCT(parse) src = MPT_PARSE_INIT;
		MPT_STRUCT(parsefmt) fmt = MPT_PARSEFMT_INIT;
		int err;
		if (!(src.src.arg = fdopen(fd, "r"))) {
			if (mt) {
				mt->_vptr->ref.unref((void *) mt);
			}
			return 0;
		}
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Debug3), "%s: %s/%s",
			        MPT_tr("process config file"), root, _file);
		}
		src.src.getc = (int (*)(void *))  mpt_getchar_stdio;
		src.src.line = 1;
		err = mpt_parse_config((MPT_TYPE(ParserFcn)) mpt_parse_format_pre, &fmt, &src, cfgSet, &ctx);
		if (err >= 0) ret |= 1;
		fclose(src.src.arg);
		
		if (err < 0) {
			int line = src.src.line;
			mpt_log(log, __func__, MPT_LOG(Info), "%s [%d] (line = %d): %s/%s",
			        MPT_tr("config file error"), err, line, root, _file);
		}
		else if (log) {
			mpt_log(log, __func__, MPT_LOG(Debug3), "%s: %s/%s",
			        MPT_tr("finished config file"), root, _file);
		}
	}
	if (mt) {
		mt->_vptr->ref.unref((void *) mt);
	}
	close(dir);
	return ret;
}
