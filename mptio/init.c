/*!
 * create control connection/socket
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <poll.h>

#include "meta.h"
#include "config.h"
#include "convert.h"
#include "parse.h"
#include "node.h"

#include "stream.h"
#include "notify.h"

#include "client.h"

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
extern int mpt_init(MPT_STRUCT(notify) *no, int argc, char * const argv[])
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(metatype) *top;
	MPT_INTERFACE(config) *cfg;
	const char *ctl = 0, *src = 0, *debug, *flags;
	int lv = 0;
	
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
	while (argc) {
		int c;
		
		switch (c = getopt(argc, argv, "+f:c:l:e:v")) {
		    case -1:
			argc = 0;
			continue;
		    case 'f':
			if ((c = loadConfig(0, optarg)) < 0) {
				break;
			}
			continue;
		    case 'c':
			if (ctl) {
				c = MPT_ERROR(BadArgument);
				break;
			}
			if (!no) {
				c = MPT_ERROR(BadOperation);
				break;
			}
			ctl = optarg;
			continue;
		    case 'l':
			if (src) {
				c = MPT_ERROR(BadArgument);
				break;
			}
			if (!no) {
				c = MPT_ERROR(BadOperation);
				break;
			}
			src = optarg;
			continue;
		    case 'v':
			setDebug(++lv);
			continue;
		    case 'e':
			setEnviron(optarg);
			continue;
		    default:
			c = MPT_ERROR(BadArgument);
		}
		if (c < 0) {
			top->_vptr->ref.unref((void *) top);
			return c;
		}
	}
	/* set executable name */
	mpt_config_set(cfg, 0, argv[0], 0, 0);
	
	/* get listen/connect target from configuration */
	if (!src && (mt = mpt_config_get(cfg, "listen", '.', 0))) {
		mt->_vptr->conv(mt, 's', &src);
	}
	if (!ctl && (mt = mpt_config_get(cfg, "connect", '.', 0))) {
		mt->_vptr->conv(mt, 's', &ctl);
	}
	top->_vptr->ref.unref((void *) top);
	
	/* wait for connection to activate input */
	if (src && (lv = mpt_notify_bind(no, src, 0)) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %s",
		        MPT_tr("unable to create source"), src);
		return lv;
	}
	/* no control channel */
	if (!ctl) {
		return optind;
	}
	/* use stdin as command source */
	if (ctl[0] == '-' && !ctl[1]) {
		static const int mode = MPT_STREAMFLAG(Read) | MPT_STREAMFLAG(Buffer);
		MPT_INTERFACE(input) *in;
		MPT_STRUCT(socket) sock;
		/* detach stdin */
		sock._id  = dup(STDIN_FILENO);
		
		if (!(in = mpt_stream_input(&sock, mode, MPT_ENUM(EncodingCommand), 0))) {
			mpt_notify_fini(no);
			close(sock._id);
			return MPT_ERROR(BadOperation);
		}
		else if ((lv = mpt_notify_add(no, POLLIN, in)) < 0) {
			mpt_notify_fini(no);
			in->_vptr->meta.ref.unref((void *) in);
			return lv;
		}
		/* detach regular input to avoid confusion */
		else {
			close(STDIN_FILENO);
		}
	}
	/* add command source */
	else if ((lv = mpt_notify_connect(no, ctl)) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %s",
		        MPT_tr("unable to connect to control"), ctl);
		mpt_notify_fini(no);
		return lv;
	}
	return optind;
}

