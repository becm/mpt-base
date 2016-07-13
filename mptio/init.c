/*!
 * create control connection/socket
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <poll.h>

#include "meta.h"
#include "config.h"

#include "array.h"
#include "convert.h"
#include "event.h"
#include "parse.h"

#include "stream.h"
#include "notify.h"

#include "output.h"

#include "client.h"

#include <signal.h>

extern MPT_STRUCT(notify) *mpt_init(int argc, char *argv[])
{
	MPT_INTERFACE(metatype) *conf;
	MPT_INTERFACE(output) *out;
	MPT_STRUCT(dispatch) *disp;
	MPT_STRUCT(notify) *no;
	const char *ctl = 0, *src = 0, *cname = 0, *log;
	int lv = 0;
	
	while (argc) {
		int c;
		
		switch (c = getopt(argc, argv, "+f:c:s:v")) {
		    case -1:
			argc = 0;
			if (cname) break;
			cname = argv[optind];
			continue;
		    case 'f':
			if (cname) return 0; cname = optarg; continue;
		    case 'c':
			if (ctl)   return 0; ctl = optarg;  continue;
		    case 's':
			if (src)   return 0; src = optarg;   continue;
		    case 'v':
			++lv; continue;
		    default:
			errno = EINVAL; return 0;
		}
	}
	if (lv) {
		lv += MPT_ENUM(LogLevelInfo);
		if (lv > MPT_ENUM(LogLevelDebug3)) {
			lv = MPT_ENUM(LogLevelDebug3);
		}
		mpt_log_default_level(lv);
	}
	else if ((log = getenv("MPT_DEBUG"))) {
		lv = MPT_ENUM(LogDebug2);
		if (mpt_cint(&lv, log, 0, 0) > 0) {
			switch (lv) {
			  case 0: lv = MPT_ENUM(LogLevelInfo); break;
			  case 1: lv = MPT_ENUM(LogLevelDebug1); break;
			  case 2: lv = MPT_ENUM(LogLevelDebug2); break;
			  case 3: lv = MPT_ENUM(LogLevelDebug3); break;
			  default: lv = MPT_ENUM(LogFile); break;
			}
			mpt_log_default_level(lv);
		}
	}
	else if ((log = getenv("MPT_LOGLEVEL"))) {
		lv = mpt_log_level(log);
		mpt_log_default_level(lv);
	}
	
	/* load mpt environment variables */
	mpt_config_environ(0, "mpt_*", '_', 0);
	mpt_config_load(getenv("MPT_PREFIX"), mpt_log_default(), 0);
	
	
	/* set client filename */
	if (cname) {
		mpt_config_set(0, "mpt.client", cname, '.', 0);
	}
	/* listen on specified address */
	if (!src && (conf = mpt_config_get(0, "mpt.client.listen", '.', 0))) {
		src = mpt_meta_data(conf, 0);
	}
	/* connect to specified address */
	if (!ctl && (conf = mpt_config_get(0, "mpt.client.connect", '.', 0))) {
		ctl = mpt_meta_data(conf, 0);
	}
	if (!(no = malloc(sizeof(*no) + sizeof(*disp)))) {
		return 0;
	}
	disp = (void *) (no + 1);
	mpt_dispatch_init(disp);
	mpt_notify_init(no);
	mpt_notify_setdispatch(no, disp);
	
	/* set notification output */
	if ((conf = mpt_output_new(no))
	    && (conf->_vptr->conv(conf, MPT_ENUM(TypeOutput), &out) >= 0)
	    && out) {
		disp->_err.arg = out;
		/* set debug parameter */
		if ((conf = mpt_config_get(0, "mpt.output.print", '.', 0))
		    && (cname = mpt_meta_data(conf, 0))) {
			mpt_object_set((void *) out, "print", "s", cname);
		}
		/* use local debug parameter */
		else if (lv > 0) {
			mpt_object_set((void *) out, "debug", "i", lv);
		}
		/* use global debug parameter */
		else if ((conf = mpt_config_get(0, "mpt.debug", '.', 0))
		    && (cname = mpt_meta_data(conf, 0))) {
			mpt_object_set((void *) out, "debug", "s", cname);
		}
		/* set answer parameter */
		if ((conf = mpt_config_get(0, "mpt.output.answer", '.', 0))
		    && (cname = mpt_meta_data(conf, 0))) {
			mpt_object_set((void *) out, "answer", "s", cname);
		}
		/* use output for reply */
		if (out->_vptr->obj.addref((void *) out)) {
			MPT_STRUCT(reply_context) *ctx;
			
			if (!(ctx = malloc(sizeof(*ctx) + 32))) {
				out->_vptr->obj.ref.unref((void *) out);
			} else {
				disp->_ctx = ctx;
				ctx->ptr = out;
				ctx->len = 0;
				ctx->_max = sizeof(ctx->_val) + 32;
				ctx->used = 0;
			}
		}
	}
	/* set default event if no input available */
	if (!src && !ctl) {
		disp->_def = mpt_hash("start", 5);
		return no;
	}
	if (ctl) {
		/* use stdin as command source */
		if (ctl[0] == '-' && !ctl[1]) {
			static const int mode = MPT_ENUM(StreamRead) | MPT_ENUM(StreamBuffer);
			MPT_INTERFACE(input) *in;
			MPT_STRUCT(socket) sock;
			/* detach stdin */
			sock._id  = dup(STDIN_FILENO);
			
			if (!(in = mpt_stream_input(&sock, mode, MPT_ENUM(EncodingCommand), 0))) {
				close(sock._id);
			}
			else if (mpt_notify_add(no, POLLIN, in) < 0) {
				in->_vptr->ref.unref((void *) in);
			}
			/* detach regular input to avoid confusion */
			else {
				close(STDIN_FILENO);
			}
		}
		/* add command source */
		else if (mpt_notify_connect(no, ctl) < 0) {
			mpt_output_log(out, __func__, MPT_FCNLOG(Error),
			               "%s: %s", MPT_tr("unable to connect to control"), ctl);
			
			mpt_notify_fini(no);
			free(no);
			return 0;
		}
	}
	/* open socket with 2 listening slots */
	if (src && mpt_notify_bind(no, src, 2) < 0) {
		mpt_output_log(out, __func__, MPT_FCNLOG(Error),
		               "%s: %s", MPT_tr("unable to create source"), src);
		
		mpt_notify_fini(no);
		free(no);
		return 0;
	}
	
	return no;
}

