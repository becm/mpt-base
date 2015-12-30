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

#include "node.h"
#include "config.h"

#include "array.h"
#include "convert.h"
#include "message.h"
#include "event.h"

#include "stream.h"
#include "notify.h"

#include "output.h"

#include "client.h"

#include <signal.h>

extern MPT_STRUCT(notify) *mpt_init(int argc, char *argv[])
{
	MPT_INTERFACE(metatype) *conf;
	MPT_INTERFACE(object) *obj;
	MPT_STRUCT(notify) *no;
	MPT_STRUCT(dispatch) *disp;
	const char *ctl = 0, *src = 0, *cname = 0;
	
	while (argc) {
		int c;
		
		switch (c = getopt(argc, argv, "+f:c:s:")) {
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
		    default:
			errno = EINVAL; return 0;
		}
	}
	
	/* load mpt environment variables */
	mpt_config_environ(0, "mpt_*", '_', 0);
	
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
	if ((disp->_out || (disp->_out = mpt_output_new(no)))
	    && (obj = disp->_out->_vptr->_mt.typecast((void *) disp->_out, MPT_ENUM(TypeObject)))) {
		/* set debug parameter */
		if ((conf = mpt_config_get(0, "mpt.output.print", '.', 0))
		    && (cname = mpt_meta_data(conf, 0))) {
			mpt_object_set(obj, "print", "s", cname);
		}
		/* set debug parameter */
		else if ((conf = mpt_config_get(0, "mpt.debug", '.', 0))
		    && (cname = mpt_meta_data(conf, 0))) {
			mpt_object_set(obj, "debug", "s", cname);
		}
		/* set answer parameter */
		if ((conf = mpt_config_get(0, "mpt.output.answer", '.', 0))
		    && (cname = mpt_meta_data(conf, 0))) {
			mpt_object_set(obj, "answer", "s", cname);
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
				in->_vptr->unref(in);
			}
			/* detach regular input to avoid confusion */
			else {
				close(STDIN_FILENO);
			}
		}
		/* add command source */
		else if (mpt_notify_connect(no, ctl) < 0) {
			mpt_output_log(disp->_out, __func__, MPT_FCNLOG(Error),
			               "%s: %s", MPT_tr("unable to connect to control"), ctl);
			
			mpt_notify_fini(no);
			free(no);
			return 0;
		}
	}
	/* open socket with 2 listening slots */
	if (src && mpt_notify_bind(no, src, 2) < 0) {
		mpt_output_log(disp->_out, __func__, MPT_FCNLOG(Error),
		               "%s: %s", MPT_tr("unable to create source"), src);
		
		mpt_notify_fini(no);
		free(no);
		return 0;
	}
	
	return no;
}

