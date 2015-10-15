/*!
 * set initial parameter for output descriptor
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include <sys/uio.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <poll.h>

#include "array.h"
#include "queue.h"
#include "event.h"
#include "convert.h"
#include "message.h"

#include "notify.h"

#include "output.h"

#define OutputFlags(a) (a & 0xf)

MPT_STRUCT(out_data) {
	MPT_INTERFACE(output) _base;
	MPT_INTERFACE(logger) _log;
	MPT_INTERFACE(input)  _in;
	uintptr_t             _shared;
	
	MPT_STRUCT(array)     _wait;
	
	MPT_STRUCT(outdata)    out;
	struct {
		MPT_TYPE(DataDecoder) fcn;
		MPT_STRUCT(codestate) info;
	} dec;
	MPT_STRUCT(notify)   *_no;
	
	MPT_STRUCT(queue)      in;
	
	struct {
		MPT_STRUCT(histinfo) info;
		FILE                *file;
	} hist;
	
	uint16_t               cid,
	                      _rid,
	                      _start;
	uint8_t               _coding;
};

static ssize_t outputPush(MPT_INTERFACE(output) *out, size_t len, const void *src)
{
	ssize_t ret;
	MPT_STRUCT(out_data) *od = (void *) out;
	
	/* message in progress */
	if (od->out.state & MPT_ENUM(OutputActive)) {
		/* local print condition */
		if (od->out.state & 0x7) {
			if (!(ret = mpt_outdata_print(&od->out, od->hist.file, len, src))) {
				ret = len;
			}
		}
		/* history output triggered */
		else if (od->hist.info.size) {
			ret = mpt_history_print(od->hist.file, &od->hist.info, len, src);
		}
		else {
			ret = mpt_outdata_push(&od->out, len, src);
		}
		if (ret < 0) {
			return ret;
		}
		if (!len) {
			od->out.state &= MPT_ENUM(OutputPrintColor);
			od->hist.info.size = 0;
			od->cid = 0;
		}
		return ret;
	}
	if (!src) {
		return -2;
	}
	if (len > 1 && !(od->out.state & MPT_ENUM(OutputRemote))) {
		const MPT_STRUCT(msgtype) *mt = src;
		if ((ret = mpt_outdata_print(&od->out, od->hist.file, len, src)) >= 0) {
			return ret ? ret : (ssize_t) len;
		}
		/* convert history to printable output */
		if (mt->cmd == MPT_ENUM(MessageValFmt)) {
			size_t parts = mt->arg;
			ret = 2 + parts * 2;
			if (len < (size_t) ret) {
				return -2;
			}
			mpt_history_set(&od->hist.info, 0);
			while (parts--) {
				if (mpt_history_set(&od->hist.info, (void *) (++mt)) < 0) {
					od->hist.info.line = 0;
					od->hist.info.type = 0;
				}
			}
			od->out.state |= MPT_ENUM(OutputActive);
			
			/* consume data for bad setup */
			if (!od->hist.info.type
			    || !od->hist.info.size
			    || ((parts = (len - ret))
			        && (ret = mpt_history_print(od->hist.file, &od->hist.info, parts, src)) < 0)) {
				od->out.state |= MPT_ENUM(OutputPrintRestore);
				return 0;
			}
			return len;
		}
	}
	/* TODO: semantics to skip ID */
	if (!(od->out.state & 0x40)) {
		struct iovec vec;
		uint16_t mid = htons(od->cid);
		
		vec.iov_base = &mid;
		vec.iov_len  = sizeof(mid);
		
		if (MPT_socket_active(&od->out.sock)
		    && (ret = mpt_array_push(&od->out._buf, &od->out._enc.info, od->out._enc.fcn, &vec)) < (ssize_t) sizeof(mid)) {
			return -3;
		}
	}
	od->out.state &= ~MPT_ENUM(OutputRemote);
	if ((ret = mpt_outdata_push(&od->out, len, src)) < 0) {
		MPT_STRUCT(command) *cmd;
		if (od->cid && (cmd = mpt_command_get(&od->_wait, od->cid))) {
			cmd->cmd(cmd->arg, 0);
			cmd->cmd = 0;
		}
	}
	return ret;
}

static int outputSync(MPT_INTERFACE(output) *out, int timeout)
{
	struct pollfd sock;
	size_t len;
	int ret;
	MPT_STRUCT(command) *ans;
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(out_data) *od = (void *) out;
	
	if ((sock.fd = od->out.sock._id) < 0) {
		errno = EBADF;
		return -1;
	}
	sock.events = POLLIN;
	
	/* count waiting handlers */
	if (!(buf = od->_wait._buf) || !(len = buf->used)) {
		return 0;
	}
	len /= sizeof(*ans);
	
	for (ret = 0, ans = (void *) (buf + 1); len; --len, ++ans) {
		if (ans->cmd) {
			++ret;
		}
	}
	if (!(len = ret)) {
		return 0;
	}
	while (1) {
		uint16_t buf[64];
		struct iovec vec;
		
		vec.iov_base = buf;
		vec.iov_len = sizeof(buf);
		if (mpt_queue_peek(&od->in, &od->dec.info, od->dec.fcn, &vec) >= 2) {
			ssize_t raw;
			
			buf[0] = ntohs(buf[0]);
			
			if (!(buf[0] & 0x8000)) {
				errno = EINTR;
				return -2;
			}
			if (!(ans = mpt_command_get(&od->_wait, buf[0] & 0x7fff))) {
				errno = EINVAL;
				return -3;
			}
			if ((raw = mpt_queue_recv(&od->in, &od->dec.info, od->dec.fcn)) >= 0) {
				MPT_STRUCT(message) msg;
				size_t off;
				
				off = od->dec.info.done;
				mpt_message_get(&od->in, off, raw, &msg, &vec);
				
				/* consume message ID */
				mpt_message_read(&msg, sizeof(*buf), 0);
				
				/* process reply and clear wait state */
				ret = ans->cmd(ans->arg, &msg);
				ans->cmd = 0;
				mpt_queue_crop(&od->in, 0, off);
				if (ret < 0) {
					return -4;
				}
				if (!--len) {
					return 0;
				}
				continue;
			}
		}
		if ((ret = poll(&sock, 1, timeout)) < 0) {
			break;
		}
		if (!(sock.revents & POLLIN)) {
			break;
		}
		
		if (od->in.len >= od->in.max && !mpt_queue_prepare(&od->in, 64)) {
			break;
		}
		if ((ret = mpt_queue_load(&od->in, od->out.sock._id, 0)) <= 0) {
			break;
		}
	}
	return len;
}

static int outputAwait(MPT_INTERFACE(output) *out, int (*ctl)(void *, const MPT_STRUCT(message) *), void *udata)
{
	MPT_STRUCT(command) *cmd;
	MPT_STRUCT(out_data) *od = (void *) out;
	
	if (!MPT_socket_active(&od->out.sock) || !(od->out._sflg & MPT_ENUM(SocketRead))) {
		return -1;
	}
	/* message in progress */
	if (od->cid || (od->out.state & MPT_ENUM(OutputActive))) {
		return -2;
	}
	if (!(cmd = mpt_message_nextid(&od->_wait))) {
		return -1;
	}
	/* make next message non-local */
	od->out.state |= MPT_ENUM(OutputRemote);
	od->cid = cmd->id;
	
	if (ctl) {
		cmd->cmd = (int (*)()) ctl;
		cmd->arg = udata;
	}
	return 1 + cmd - ((MPT_STRUCT(command) *) (od->_wait._buf + 1));
}
static int outputUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(out_data) *odata = (void *) mt;
	FILE *fd;
	if (odata->_shared) {
		return odata->_shared--;
	}
	mpt_outdata_fini(&odata->out);
	mpt_command_clear(&odata->_wait);
	
	free(odata->in.base);
	
	mpt_history_setfmt(&odata->hist.info, 0);
	
	if ((fd = odata->hist.file)
	    && (fd != stdin)
	    && (fd != stdout)
	    && (fd != stderr)) {
		fclose(fd);
		odata->hist.file = 0;
	}
	
	free(mt);
	return 0;
}
static MPT_INTERFACE(metatype) *outputRef(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(out_data) *od = (void *) mt;
	if (++od->_shared) return mt;
	--od->_shared;
	return 0;
}

static void *outputCast(MPT_INTERFACE(metatype) *mt, int type) {
	MPT_STRUCT(out_data) *od = (void *) mt;
	
	switch (type) {
	  case MPT_ENUM(TypeMeta): return mt;
	  case MPT_ENUM(TypeLogger): return &od->_log;
	  case MPT_ENUM(TypeOutput): return &od->_base;
	  case MPT_ENUM(TypeInput):  return &od->_in;
	  default: return 0;
	}
}

static int setHistfile(FILE **hist, MPT_INTERFACE(source) *src)
{
	const char *where = 0;
	int len;
	FILE *fd;
	
	if (!src) {
		return *hist ? 1 : 0;
	}
	if ((len = src->_vptr->conv(src, 's', &where)) < 0) {
		return len;
	} else if (!where) {
		fd = stdout;
	} else if (!*where) {
		fd = 0;
	} else {
		MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
		int mode;
		
		/* try to use argument as connect string */
		if ((mode = mpt_connect(&sock, where, 0)) >= 0) {
			if (!(mode & MPT_ENUM(SocketStream))
			    || !(mode & MPT_ENUM(SocketWrite))
			    || !(fd = fdopen(sock._id, "w"))) {
				mpt_connect(&sock, 0, 0);
				return -1;
			}
		}
		/* regular file path */
		else if (!(fd = fopen(where, "w"))) {
			return -1;
		}
	}
	if (*hist && (*hist != stdout) && (*hist != stderr)) {
		fclose(*hist);
	}
	*hist = fd;
	
	return len;
}
static int outputEncoding(MPT_STRUCT(out_data) *od, MPT_INTERFACE(source) *src)
{
	MPT_TYPE(DataEncoder) enc;
	MPT_TYPE(DataDecoder) dec;
	char *where;
	int32_t val;
	int type;
	uint8_t rtype;
	
	if (!src) return od->_coding;
	
	if (MPT_socket_active(&od->out.sock)) {
		return -4;
	}
	if ((type = src->_vptr->conv(src, 's', &where)) >= 0) {
		val = mpt_encoding_value(where, -1);
		if (val < 0 || val > UINT8_MAX) {
			return -2;
		}
		rtype = val;
	}
	else if ((type = src->_vptr->conv(src, 'B', &rtype)) < 0) {
		type = src->_vptr->conv(src, 'i', &val);
		if (type < 0 || val < 0 || val > UINT8_MAX) {
			return -3;
		}
		rtype = val;
	}
	if (!rtype) {
		if (od->out._enc.fcn) {
			od->out._enc.fcn(&od->out._enc.info, 0, 0);
			od->out._enc.fcn = 0;
		}
		if (od->dec.fcn) {
			od->dec.fcn(&od->dec.info, 0, 0);
			od->dec.fcn = 0;
		}
	}
	else if (!(enc = mpt_message_encoder(rtype))
	    || !(dec = mpt_message_decoder(rtype))) {
		return -3;
	}
	else {
		od->out._enc.fcn = enc;
		od->dec.fcn = dec;
	}
	od->_coding = rtype;
	
	return type;
}
static int outputProp(MPT_INTERFACE(metatype) *mt, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	MPT_STRUCT(out_data) *odata = (void *) mt;
	MPT_STRUCT(outdata) *od = &odata->out;
	const char *name;
	int ret, oldFd = od->sock._id;
	
	if (!prop) {
		if (!src) {
			return MPT_ENUM(TypeOutput);
		}
		return mpt_outdata_property(od, prop, src);
	}
	if (!(name = prop->name)) {
		return src ? -1 : -3;
	}
	if (!strcasecmp(name, "history") || !strcasecmp(name, "histfile")) {
		if ((ret = setHistfile(&odata->hist.file, src)) < 0) {
			return ret;
		}
		prop->name = "history";
		prop->desc = MPT_tr("history data output file");
		prop->val.fmt = "";
		prop->val.ptr = odata->hist.file;
		return ret;
	}
	if (!strcasecmp(name, "histfmt")) {
		if (!src) {
			ret = odata->hist.info.fmt ? 1 : 0;
		}
		else if ((ret = mpt_history_setfmt(&odata->hist.info, src)) < 0) {
			return ret;
		}
		prop->name = "histfmt";
		prop->desc = "history data output format";
		prop->val.fmt = "";
		prop->val.ptr = odata->hist.info.fmt;
		return ret;
	}
	if (!strcasecmp(name, "encoding")) {
		if ((ret = outputEncoding(odata, src)) < 0) {
			return ret;
		}
		prop->name = "encoding";
		prop->desc = "socket stream encoding";
		prop->val.fmt = "B";
		prop->val.ptr = &odata->_coding;
		return ret;
	}
	if ((ret = mpt_outdata_property(od, prop, src)) < 0) {
		mpt_log(&odata->_log, __func__, MPT_FCNLOG(Debug), "%s: %s",
		        src ? MPT_tr("unable to set property") : MPT_tr("invalid property"),
		        prop->name);
		return ret;
	}
	/* conditions for notification change */
	if (prop->val.ptr == &od->sock) {
		if (od->_enc.fcn) {
			od->_enc.fcn(&od->_enc.info, 0, 0);
			od->_enc.fcn = 0;
		}
		if (odata->dec.fcn) {
			odata->dec.fcn(&odata->dec.info, 0, 0);
			odata->dec.fcn = 0;
		}
		if (od->_sflg & MPT_ENUM(SocketStream)) {
			if (!odata->_coding) {
				odata->_coding = MPT_ENUM(EncodingCobs);
			}
			od->_enc.fcn = mpt_message_encoder(odata->_coding);
			odata->dec.fcn = mpt_message_decoder(odata->_coding);
		}
		if (!odata->_no || (od->sock._id == oldFd)) {
			return ret;
		}
		/* remove old registration */
		if (oldFd > 2) {
			mpt_notify_clear(odata->_no, oldFd);
		}
		if (!MPT_socket_active(&od->sock)) {
			return ret;
		}
		
		/* add local reference for event controller */
		if (!outputRef((void *) odata)) {
			mpt_log(&odata->_log, __func__, MPT_FCNLOG(Error), "%s: %s "PRIxPTR,
			        MPT_tr("failed"),
			        MPT_tr("reference output"),
			        od);
		}
		/* use first reference for notifier */
		else if (mpt_notify_add(odata->_no, POLLIN, &odata->_in) < 0) {
			mpt_log(&odata->_log, __func__, MPT_FCNLOG(Error), "%s: %s: fd%i",
			        MPT_tr("failed"),
			        MPT_tr("register notifier"),
			        (int) od->sock._id);
			/* clear references */
			outputUnref((void *) odata);
		}
	}
	return ret;
}

static const MPT_INTERFACE_VPTR(output) outCtl = {
	{ outputUnref, outputRef, outputProp, outputCast },
	outputPush,
	outputSync,
	outputAwait
};

static int outputLoggerUnref(MPT_INTERFACE(logger) *log)
{
	return outputUnref(MPT_reladdr(out_data, log, _log, _base));
}

static int outputLog(MPT_INTERFACE(logger) *log, const char *from, int type, const char *fmt, va_list va)
{
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, log, _log, _base);
	MPT_STRUCT(outdata) *od = &odata->out;
	FILE *fd;
	const char *ansi = 0;
	
	/* send log entry to contact */
	if (!(od->state & MPT_ENUM(OutputActive))
	    && (od->state & MPT_ENUM(OutputRemote))) {
		MPT_STRUCT(msgtype) hdr;
		
		hdr.cmd = MPT_ENUM(MessageOutput);
		hdr.arg = type & 0xff;
		
		outputPush(&odata->_base, sizeof(hdr), &hdr);
		
		if (type && from) {
			outputPush(&odata->_base, strlen(from)+1, from);
		}
		if (fmt) {
			char buf[1024];
			int plen;
			
			plen = vsnprintf(buf, sizeof(buf), fmt, va);
			
			/* zero termination indicates truncation,
			 * just ignore Microsoft's fuckuped version without termination */
			if (plen >= (int) sizeof(buf)) plen = sizeof(buf);
			
			if (plen > 0 && (plen = outputPush(&odata->_base, plen, buf)) < 0) {
				outputPush(&odata->_base, 1, 0);
				return plen;
			}
		}
		outputPush(&odata->_base, 0, 0);
		
		return 2;
	}
	/* local processing of log entry */
	if (!(type & 0xff)) {
		type &= ~MPT_ENUM(LogPretty);
		fd = stdout;
		fputc('#', fd);
		fputc(' ', fd);
	}
	else {
		fd = stderr;
		if ((type & MPT_ENUM(LogFile)) && odata->hist.file) {
			fd = odata->hist.file;
			type &= ~MPT_ENUM(LogFile);
		}
		else if (mpt_output_file(type & 0x7f, od->level & 0xf) <= 0) {
			return 0;
		}
		/* use default log config */
		if (!(type & MPT_ENUM(LogPretty))) {
			type |= MPT_ENUM(LogPrefix);
			if (od->state & MPT_ENUM(OutputPrintColor)) {
				type |= MPT_ENUM(LogSelect);
			}
		}
	}
	ansi = mpt_log_start(fd, from, type);
	if (fmt) {
		vfprintf(fd, fmt, va);
	}
	if (ansi) fputs(ansi, fd);
	fputc('\n', fd);
	fflush(fd);
	
	return 1;
}
static const MPT_INTERFACE_VPTR(logger) logCtl = {
	outputLoggerUnref,
	outputLog
};

static int outputInputUnref(MPT_INTERFACE(input) *in)
{
	return outputUnref(MPT_reladdr(out_data, in, _in, _base));
}
static int outputNext(MPT_INTERFACE(input) *in, int what)
{
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, in, _in, _base);
	
	if (!MPT_socket_active(&odata->out.sock)) {
		return -3;
	}
	if (what & POLLIN) {
		ssize_t len;
		
		if (odata->in.len >= odata->in.max && !mpt_queue_prepare(&odata->in, 64)) {
			return 0;
		}
		if ((len = mpt_queue_load(&odata->in, odata->out.sock._id, 0)) > 0) {
			return POLLIN;
		}
		close(odata->out.sock._id);
		odata->out.sock._id = -1;
		
		if (!odata->in.len) {
			return -3;
		}
	}
	/* dispatch dereference to notifier */
	return (what & POLLHUP) ? -2 : 0;
}

extern int replySet(void *con, const MPT_STRUCT(message) *src)
{
	static const char fcn[] = "mpt::output::reply";
	MPT_STRUCT(out_data) *rep = con;
	struct iovec vec;
	ssize_t take;
	uint16_t rid;
	
	/* already answered */
	if (!rep->_rid) {
		mpt_log(&rep->_log, fcn, MPT_FCNLOG(Warning), "%s",
		        MPT_tr("reply to processed message ignored"));
		return -3;
	}
	
	if (rep->out.state & MPT_ENUM(OutputActive)) {
		mpt_log(&rep->_log, fcn, MPT_FCNLOG(Error), "%s (%04x): %s",
	        MPT_tr("unable to reply"), rep->_rid, MPT_tr("message in progress"));
		rep->_rid = 0;
		return -1;
	}
	rid = htons(rep->_rid);
	vec.iov_base = &rid;
	vec.iov_len  = sizeof(rid);
	
	if (mpt_array_push(&rep->out._buf, &rep->out._enc.info, rep->out._enc.fcn, &vec) < (ssize_t) sizeof(rid)) {
		mpt_log(&rep->_log, fcn, MPT_FCNLOG(Warning), "%s (%04x)",
		        MPT_tr("error replying to message"), rep->_rid);
		rep->_rid = 0;
		return -1;
	}
	rep->out.state |= MPT_ENUM(OutputActive);
	
	if (src) {
		struct iovec *cont = src->cont;
		size_t clen = src->clen;
		
		vec.iov_base = cont->iov_base;
		vec.iov_len  = cont->iov_len;
		
		
		
		while (1) {
			if (!vec.iov_len) {
				if (!--clen) {
					break;
				}
				vec = *(cont++);
				
				continue;
			}
			take = mpt_outdata_push(&rep->out, vec.iov_len, vec.iov_base);
			
			if (take < 0 || (size_t) take > vec.iov_len) {
				if ((take = mpt_outdata_push(&rep->out, 1, 0)) < 0) {
					return take;
				}
				return -1;
			}
			vec.iov_base = ((uint8_t *) vec.iov_base) + take;
			vec.iov_len -= take;
			
			/* mark written data */
			rid = 0;
		}
	}
	if ((take = mpt_outdata_push(&rep->out, 0, 0)) < 0) {
		if (!rid) {
			(void) mpt_outdata_push(&rep->out, 1, 0);
		}
		take = -1;
	}
	rep->out.state &= ~(MPT_ENUM(OutputActive) | MPT_ENUM(OutputRemote));
	
	return take;
}
static int outputDispatch(MPT_INTERFACE(input) *in, MPT_TYPE(EventHandler) cmd, void *arg)
{
	MPT_STRUCT(event) ev;
	MPT_STRUCT(message) msg;
	struct iovec vec;
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, in, _in, _base);
	MPT_STRUCT(outdata) *od = &odata->out;
	ssize_t len;
	size_t off;
	int ret;
	uint16_t rid;
	
	/* message trnasfer in progress */
	if (od->state & MPT_ENUM(OutputActive)) {
		return MPT_ENUM(EventRetry);
	}
	if (!odata->dec.fcn) {
		errno = EINVAL;
		return -1;
	}
	
	/* get next message */
	if ((len = mpt_queue_recv(&odata->in, &odata->dec.info, odata->dec.fcn)) < 0) {
		return len;
	}
	off = odata->dec.info.done;
	mpt_message_get(&odata->in, off, len, &msg, &vec);
	
	/* remove message id */
	if (mpt_message_read(&msg, sizeof(rid), &rid) < sizeof(rid)) {
		if (off) {
			mpt_queue_crop(&odata->in, 0, off);
			odata->dec.info.done = 0;
		}
		return -2;
	}
	rid = ntohs(rid);
	
	/* process reply */
	if (rid & 0x8000) {
		MPT_STRUCT(command) *ans;
		
		rid &= 0x7fff;
		ev.id = rid;
		ev.msg = 0;
		ev.reply.set = 0;
		ev.reply.context = 0;
		
		if ((ans = mpt_command_get(&odata->_wait, rid))) {
			if (ans->cmd(ans->arg, &msg) < 0) {
				mpt_log(&odata->_log, __func__, MPT_FCNLOG(Warning), "%s: %04x",
				        MPT_tr("reply processing error"), rid);
			}
			ans->cmd = 0;
		} else {
			ev.msg = &msg;
			mpt_log(&odata->_log, __func__, MPT_FCNLOG(Error), "%s: %04x",
			        MPT_tr("unregistered reply id"), rid);
		}
	}
	/* process regular input */
	else {
		/* event setup */
		ev.id = 0;
		ev.msg = &msg;
		
		/* force remote message */
		if (rid) {
			odata->_rid = rid | 0x8000;
			odata->_start = 0;
			ev.reply.set = replySet;
			ev.reply.context = odata;
		}
	}
	/* dispatch data to command */
	if (!cmd) {
		ret = 0;
	}
	else if ((ret = cmd(arg, &ev)) < 0) {
		ret = MPT_ENUM(EventCtlError);
	} else {
		ret &= MPT_ENUM(EventFlags);
	}
	if (odata->_rid) {
		replySet(odata, 0);
	}
	/* remove message data from queue */
	if (off) {
		mpt_queue_crop(&odata->in, 0, off);
		odata->dec.info.done = 0;
	}
	/* further message on queue */
	if ((len = mpt_queue_peek(&odata->in, &odata->dec.info, odata->dec.fcn, 0)) >= 0) {
		ret |= MPT_ENUM(EventRetry);
	}
	return ret;
}
static int outputFile(MPT_INTERFACE(input) *in)
{
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, in, _in, _base);
	return odata->out.sock._id;
}
const MPT_INTERFACE_VPTR(input) inputCtl = {
	outputInputUnref,
	
	outputNext,
	outputDispatch,
	outputFile
};

/*!
 * \ingroup mptOutput
 * \brief create output
 * 
 * New output descriptor.
 * 
 * If notification descriptor is passed
 * input on notifier is updated on connection change.
 * 
 * Passed notifier must be available for output livetime.
 * 
 * \param no notification descriptor
 * 
 * \return output descriptor
 */
extern MPT_INTERFACE(output) *mpt_output_new(MPT_STRUCT(notify) *no)
{
	static const MPT_STRUCT(out_data) defOut = {
		{ &outCtl }, { &logCtl }, { &inputCtl }, 0,
		MPT_ARRAY_INIT,
		MPT_OUTDATA_INIT, { 0, MPT_CODESTATE_INIT }, 0,
		MPT_QUEUE_INIT,
		{ MPT_HISTINFO_INIT, 0 },
		0, 0, 0,
		0
	};
	MPT_STRUCT(out_data) *odata;
	
	if (!(odata = malloc(sizeof(*odata)))) {
		return 0;
	}
	*odata = defOut;
	
	odata->out.state = MPT_ENUM(OutputPrintColor);
	odata->out.level = (MPT_ENUM(OutputLevelWarning) << 4) | MPT_ENUM(OutputLevelWarning);
	
	odata->_no = no;
	
	return &odata->_base;
}
