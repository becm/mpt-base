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
#include "output.h"
#include "types.h"

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
	if (MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &cfg) < 0
	    || !cfg) {
		return;
	}
	cfg->_vptr->remove(cfg, 0);
}

static void setDebug(int lv)
{
	static int old = -1;
	
	/* maximize log level */
	if (old >= lv) {
		return;
	}
	old = lv;
	/* set log skip begin */
	lv = MPT_LOG(Debug) + lv * (MPT_LOG(Debug2) - MPT_LOG(Debug));
	if (lv > MPT_LOG(File)) {
		lv = MPT_LOG(File);
	}
	mpt_log_default_skip(lv);
}
static void setEnviron(const char *match)
{
	static int global = 0;
	if (global) {
		return;
	}
	if (!match) {
		global = 1;
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
	MPT_STRUCT(parser_context) p = MPT_PARSER_INIT;
	MPT_STRUCT(parser_format) fmt = MPT_PARSER_FORMAT_INIT;
	int ret;
	
	if (!cfg) {
		MPT_INTERFACE(metatype) *mt = mpt_config_global(0);
		MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &cfg);
	}
	if (!(p.src.arg = fopen(fname, "r"))) {
		mpt_log(0, "mpt_init::config", MPT_LOG(Error), "%s: %s",
		        MPT_tr("failed to open config file"), fname);
		return MPT_ERROR(BadArgument);
	}
	if (!(p.src.arg = fopen(fname, "r"))) {
		return 0;
	}
	/* allow default format override */
	mpt_parse_format(&fmt, 0);
	p.src.getc = (int (*)(void *))  mpt_getchar_stdio;
	p.src.line = 1;
	ret = mpt_parse_config((MPT_TYPE(input_parser)) mpt_parse_format_pre, &fmt, &p, saveGlobal, cfg);
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
	MPT_metatype_convert(top, MPT_ENUM(TypeNodePtr), &mpt);
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
	a._buf->_content_traits = mpt_type_traits('c');
	b = mpt_meta_buffer(&a);
	mpt_array_clone(&a, 0);
	
	if (!b) {
		return MPT_ERROR(BadOperation);
	}
	if (!(c = mpt_node_find(mpt, "args", -1))) {
		if (!(c = mpt_node_new(5))) {
			b->_vptr->unref(b);
			return MPT_ERROR(BadArgument);
		}
		mpt_identifier_set(&c->ident, "args", -1);
		mpt_node_insert(mpt, 1, c);
	}
	else if ((old = c->_meta)) {
		old->_vptr->unref(old);
	}
	c->_meta = b;
	
	return 0;
}
/*!
 * \ingroup mptNotify
 * \brief initialize MPT environment
 * 
 * Load configuration and set notification source.
 * 
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
		int dbg = 1;
		if (mpt_cint(&dbg, debug, 0, 0) < 0) {
			dbg = 1;
		}
		else if (dbg > 0xe) {
			dbg = 0xf;
		}
		setDebug(dbg);
	}
	/* use global mpt config section */
	mpt_path_set(&p, "mpt", -1);
	if (!(top = mpt_config_global(&p))) {
		return MPT_ERROR(BadOperation);
	}
	MPT_metatype_convert(top, MPT_ENUM(TypeConfigPtr), &cfg);
	
	/* load configs in `etc` subdirectory */
	mpt_config_load(cfg, getenv("MPT_PREFIX_ETC"), mpt_log_default());
	
	/* additional flags from enfironment */
	if ((flags = getenv("MPT_FLAGS"))) {
		int env = 0;
		char curr;
		while ((curr = *flags++)) {
			switch (curr) {
			  case 'v':
				setDebug(lv++);
				continue;
			  case 'E':
				/* global environment loading (masks granular) */
				if (env) {
					mpt_log(0, __func__, MPT_LOG(Warning), "%s",
					        MPT_tr("global environment load after specialized"));
				}
				setEnviron(0);
				continue;
			  case 'e':
				/* limited environment on first occurance only */
				if (!env++) {
					setEnviron("mpt_*");
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
	lv = 0; /* reset debug level */
	while (optind < argc) {
		switch (ret = getopt(argc, argv, "+f:c:l:Ee:v")) {
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
			setDebug(lv++);
			continue;
		    case 'E':
			setEnviron(0);
			continue;
		    case 'e':
			setEnviron(optarg);
			continue;
		    default:
			ret = MPT_ERROR(BadArgument);
		}
		break;
	}
	top->_vptr->unref(top);
	atexit(clearConfig);
	
	return ret < 0 ? ret : optind;
}
