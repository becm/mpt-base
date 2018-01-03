/*!
 * apply initial settings
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <poll.h>

#include "array.h"
#include "meta.h"
#include "config.h"
#include "convert.h"

#include "parse.h"
#include "node.h"

static void clearConfig(void)
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(config) *cfg;
	
	if (!(mt = mpt_config_global(0))) {
		return;
	}
	cfg = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg) < 0
	    || !cfg) {
		return;
	}
	cfg->_vptr->remove(cfg, 0);
}

static void setDebug(int lv)
{
	static int old = 0;
	
	/* maximize log level */
	if (old >= lv) {
		return;
	}
	old = lv;
	/* set log skip begin */
	lv = MPT_LOG(Debug) + (lv - 1) * (MPT_LOG(Debug2) - MPT_LOG(Debug));
	if (lv > MPT_LOG(File)) {
		lv = MPT_LOG(File);
	}
	mpt_log_default_skip(lv);
}
static void setEnviron(const char *match)
{
	char buf[128];
	
	if (match) {
		int len = snprintf(buf, sizeof(buf), "%s_*", match);
		if (len < 0 || len > (int) sizeof(buf)) {
			mpt_log(0, "mpt_init::environ", MPT_LOG(Error), "%s",
			        MPT_tr("bad match format"));
		}
		match = buf;
	}
	mpt_config_environ(0, match, '_', 0);
}
/* save config elements */
static int saveGlobal(void *ptr, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val, int last, int curr)
{
	MPT_INTERFACE(config) *cfg = ptr;
	
	if ((curr & 0x3) != MPT_PARSEFLAG(Option)) {
		return 0;
	}
	if ((last = cfg->_vptr->assign(cfg, p, val)) < 0) {
		mpt_log(0, "mpt_init::TypeConfig", MPT_LOG(Error), "%s",
		        MPT_tr("failed to set global config element"));
		return last;
	}
	return 0;
}
static int loadConfig(MPT_INTERFACE(config) *cfg, const char *fname)
{
	MPT_STRUCT(parse) p = MPT_PARSE_INIT;
	MPT_STRUCT(parsefmt) fmt = MPT_PARSEFMT_INIT;
	int ret;
	
	if (!cfg) {
		MPT_INTERFACE(metatype) *mt = mpt_config_global(0);
		mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg);
	}
	if (!(p.src.arg = fopen(fname, "r"))) {
		mpt_log(0, "mpt_init::config", MPT_LOG(Error), "%s: %s",
		        MPT_tr("failed to open config file"), fname);
		return MPT_ERROR(BadArgument);
	}
	if (!(p.src.arg = fopen(fname, "r"))) {
		return 0;
	}
	p.src.getc = (int (*)(void *))  mpt_getchar_stdio;
	p.src.line = 1;
	ret = mpt_parse_config((MPT_TYPE(ParserFcn)) mpt_parse_format_pre, &fmt, &p, saveGlobal, cfg);
	fclose(p.src.arg);
	if (ret < 0) {
		int line = p.src.line;
		mpt_log(0, "mpt_init::config", MPT_LOG(Error), "%s %d (line %d): %s",
		        MPT_tr("parse error"), ret, line, fname);
		return MPT_ERROR(BadArgument);
	}
	return ret;
}
static int saveArgs(MPT_INTERFACE(metatype) *top, int argc, char * const argv[])
{
	MPT_STRUCT(array) a = MPT_ARRAY_INIT;
	MPT_INTERFACE(metatype) *b, *old;
	MPT_STRUCT(node) *mpt, *c;
	
	mpt = 0;
	top->_vptr->conv(top, MPT_ENUM(TypeNode), &mpt);
	if (!mpt) {
		return MPT_ERROR(BadType);
	}
	while (argc-- > 0) {
		const char *curr = *argv++;
		if (!curr) {
			curr = mpt_array_append(&a, 1, 0);
		} else {
			curr = mpt_array_append(&a, strlen(curr) + 1, curr);
		}
		if (!curr) {
			mpt_array_clone(&a, 0);
			return MPT_ERROR(BadOperation);
		}
	}
	b = mpt_meta_buffer(&a);
	mpt_array_clone(&a, 0);
	
	if (!b) {
		return MPT_ERROR(BadOperation);
	}
	if (!(c = mpt_node_find(mpt, "args", -1))) {
		if (!(c = mpt_node_new(5))) {
			b->_vptr->ref.unref((void *) b);
			return MPT_ERROR(BadArgument);
		}
		mpt_identifier_set(&c->ident, "args", -1);
		mpt_node_insert(mpt, 1, c);
	}
	else if ((old = c->_meta)) {
		old->_vptr->ref.unref((void *) old);
	}
	c->_meta = (void *) b;
	
	return 0;
}
/*!
 * \ingroup mptNotify
 * \brief initialize MPT environment
 * 
 * Set notification inputs and config file.
 * 
 * \param no   notification descriptor
 * \param argc argument count
 * \param argv command line arguments
 */
extern int mpt_init(int argc, char * const argv[])
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_INTERFACE(metatype) *top;
	MPT_INTERFACE(config) *cfg;
	const char *ctl = 0, *src = 0, *debug, *flags;
	int lv = 0, ret;
	
	if ((debug = getenv("MPT_DEBUG"))) {
		int dbg = 0;
		if (mpt_cint(&dbg, debug, 0, 0) < 0
		    || dbg < 0) {
			dbg = 1;
		}
		else if (dbg > 0xe) {
			dbg = 0xe;
		}
		setDebug(dbg + 1);
	}
	/* use global mpt config section */
	mpt_path_set(&p, "mpt", -1);
	if (!(top = mpt_config_global(&p))) {
		return MPT_ERROR(BadOperation);
	}
	top->_vptr->conv(top, MPT_ENUM(TypeConfig), &cfg);
	
	/* load configs in `etc` subdirectory */
	mpt_config_load(cfg, getenv("MPT_PREFIX"), mpt_log_default());
	
	/* additional flags from enfironment */
	if ((flags = getenv("MPT_FLAGS"))) {
		int env = 0;
		char curr;
		while ((curr = *flags++)) {
			switch (curr) {
			  case 'v':
				setDebug(++lv);
				continue;
			  case 'e':
				/* environment arguments on first occurance only */
				if (!env++) {
					setEnviron(0);
				}
				continue;
			  default:
				mpt_log(0, __func__, MPT_LOG(Debug), "%s, %c",
				        MPT_tr("unknown operation flag"), curr);
			}
		}
	}
	/* set executable name */
	mpt_config_set(cfg, 0, argv[0], 0, 0);
	
	ret = 0;
	while (optind < argc) {
		switch (ret = getopt(argc, argv, "+f:c:l:e:v")) {
		    case -1:
			ret = saveArgs(top, argc - optind, argv + optind);
			break;
		    case 'f':
			if ((ret = loadConfig(0, optarg)) < 0) {
				break;
			}
			continue;
		    case 'c':
			if (ctl) {
				ret = MPT_ERROR(BadArgument);
				break;
			}
			ctl = optarg;
			if ((ret = mpt_config_set(cfg, "connect", ctl, 0, 0)) < 0) {
				break;
			}
			continue;
		    case 'l':
			if (src) {
				ret = MPT_ERROR(BadArgument);
				break;
			}
			src = optarg;
			if ((ret = mpt_config_set(cfg, "listen", src, 0, 0)) < 0) {
				break;
			}
			continue;
		    case 'v':
			setDebug(++lv);
			continue;
		    case 'e':
			setEnviron(optarg);
			continue;
		    default:
			ret = MPT_ERROR(BadArgument);
		}
		break;
	}
	top->_vptr->ref.unref((void *) top);
	atexit(clearConfig);
	
	return ret < 0 ? ret : optind;
}

