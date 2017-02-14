/*!
 * create control connection/socket
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>

#include <unistd.h>
#include <poll.h>

#include "convert.h"
#include "parse.h"

#include "stream.h"
#include "notify.h"

#include "client.h"

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
	const char *ctl = 0, *src = 0, *cname = 0, *debug, *flags;
	int32_t lv = 0, env = 0;
	
	if ((debug = getenv("MPT_DEBUG"))) {
		lv = MPT_LOG(Debug2);
		if (mpt_cint(&lv, debug, 0, 0) > 0) {
			switch (lv) {
			  case 0: lv = MPT_LOG(Info); break;
			  case 1: lv = MPT_LOG(Debug)  + 0x10; break;
			  case 2: lv = MPT_LOG(Debug2) + 0x10; break;
			  case 3: lv = MPT_LOG(Debug3) + 0x10; break;
			  default: lv = 0x80; break;
			}
			mpt_log_default_skip(lv);
		}
	}
	if ((flags = getenv("MPT_FLAGS"))) {
		char curr;
		while ((curr = *flags++)) {
			switch (curr) {
			  case 'v': ++lv; break;
			  case 'e': ++env; break;
			  default:
				mpt_log(0, __func__, MPT_LOG(Debug), "%s, %c",
				        MPT_tr("unknown operation flag"), curr);
			}
		}
	}
	while (argc) {
		int c;
		
		switch (c = getopt(argc, argv, "+f:c:l:ve")) {
		    case -1:
			argc = 0;
			if (cname) break;
			cname = argv[optind];
			continue;
		    case 'f':
			if (cname) {
				return MPT_ERROR(BadArgument);
			}
			cname = optarg;
			continue;
		    case 'c':
			if (ctl) {
				return MPT_ERROR(BadArgument);
			}
			if (!no) {
				return MPT_ERROR(BadOperation);
			}
			ctl = optarg;
			continue;
		    case 'l':
			if (src) {
				return MPT_ERROR(BadArgument);
			}
			if (!no) {
				return MPT_ERROR(BadOperation);
			}
			src = optarg;
			continue;
		    case 'v':
			++lv; continue;
		    case 'e':
			++env; continue;
		    default:
			return MPT_ERROR(BadArgument);
		}
	}
	/* overwrite environment settings */
	if (lv) {
		lv = MPT_LOG(Debug) + (lv-1) * (MPT_LOG(Debug) - MPT_LOG(Debug2));
		if (lv > MPT_LOG(File)) {
			lv = MPT_LOG(File);
		}
		mpt_log_default_skip(lv);
	}
	/* load mpt environment variables */
	mpt_config_environ(0, "mpt_*", '_', 0);
	mpt_config_load(getenv("MPT_PREFIX"), mpt_log_default(), 0);
	
	/* set executable name */
	mpt_config_set(0, "mpt", argv[0], 0, 0);
	
	/* set client filename */
	if (cname) {
		mpt_config_set(0, "mpt.config", cname, '.', 0);
	}
	/* get arguments from environment */
	if (env) {
		if (!src) src = getenv("MPT_LISTEN");
		if (!ctl) ctl = getenv("MPT_CONNECT");
	}
	/* set default event if no input available */
	if (!src && !ctl) {
		return 0;
	}
	/* wait for connection to activate input */
	if (src && mpt_notify_bind(no, src, 0) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %s",
		        MPT_tr("unable to create source"), src);
		return 0;
	}
	/* no control channel */
	if (!ctl) {
		return src ? 1 : 0;
	}
	/* use stdin as command source */
	if (ctl[0] == '-' && !ctl[1]) {
		static const int mode = MPT_STREAMFLAG(Read) | MPT_STREAMFLAG(Buffer);
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
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %s",
		        MPT_tr("unable to connect to control"), ctl);
		
		mpt_notify_fini(no);
		return 0;
	}
	return src ? 2 : 1;
}

