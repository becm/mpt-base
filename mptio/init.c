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
static int loadConfig(const char *fname)
{
	MPT_STRUCT(node) *root;
	MPT_STRUCT(value) val;
	FILE *fd;
	int ret;
	
	if (!(fd = fopen(fname, "r"))) {
		mpt_log(0, "mpt_init::load", MPT_LOG(Error), "%s",
		        MPT_tr("failed to set global config element"));
		return MPT_ERROR(BadArgument);
	}
	if (!(root = mpt_config_node(0))) {
		return MPT_ERROR(BadOperation);
	}
	val.fmt = 0;
	val.ptr = fname;
	mpt_node_set(root, &val);
	ret = mpt_node_read(root, fd, 0, 0, 0);
	fclose(fd);
	if (ret < 0) {
		errno = EINVAL;
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
	const MPT_INTERFACE(metatype) *mt;
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
	/* load mpt config from `etc` directory */
	mpt_config_load(getenv("MPT_PREFIX"), mpt_log_default(), 0);
	
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
			if ((c = loadConfig(optarg)) < 0) {
				return c;
			}
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
			setDebug(++lv);
			continue;
		    case 'e':
			setEnviron(optarg);
			continue;
		    default:
			return MPT_ERROR(BadArgument);
		}
	}
	/* set executable name */
	mpt_config_set(0, "mpt", argv[0], 0, 0);
	
	/* get listen/connect target from configuration */
	if (!src && (mt = mpt_config_get(0, "mpt.listen", '.', 0))) {
		mt->_vptr->conv(mt, 's', &src);
	}
	if (!ctl && (mt = mpt_config_get(0, "mpt.connect", '.', 0))) {
		mt->_vptr->conv(mt, 's', &ctl);
	}
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
			in->_vptr->ref.unref((void *) in);
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

