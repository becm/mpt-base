/*!
 * MPT C++ stream I/O operations
 */

#include <inttypes.h>

#include <cstdio>
#include <limits>

#include <sys/uio.h>

#include "message.h"

#include "../mptio/stream.h"
#include "../mptio/connection.h"

#include "io.h"

__MPT_NAMESPACE_BEGIN

template <> int typeinfo<io::stream>::id()
{
	static int id;
	if (!id && (id = mpt_type_meta_new("stream")) < 0) {
		id = mpt_type_meta_new(0);
	}
	return id;
}

// stream class
io::stream::stream(const streaminfo *from) : _srm(0), _cid(0), _inputFile(-1), _idlen(0)
{
	if (!from) {
		return;
	}
	_srm = new ::mpt::stream;
	_mpt_stream_setfile(&_srm->_info, _mpt_stream_fread(from), _mpt_stream_fwrite(from));
	int flags = mpt_stream_flags(from);
	mpt_stream_setmode(_srm, flags & 0xff);
	_srm->set_newline(MPT_stream_newline_read(flags),  _srm->Read);
	_srm->set_newline(MPT_stream_newline_write(flags), _srm->Write);
}
io::stream::~stream()
{
	if (_srm) delete _srm;
}

// reference interface
void io::stream::unref()
{
	delete this;
}
// metatype interface
int io::stream::conv(int type, void *ptr) const
{
	int me = mpt_input_typeid();
	
	if (me < 0) {
		me = output::Type;
	}
	else if (type == to_pointer_id(me)) {
		if (ptr) *static_cast<const input **>(ptr) = this;
		return socket::Type;
	}
	if (!type) {
		static const uint8_t fmt[] = { output::Type, socket::Type, 0 };
		if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
		return me;
	}
	if (type == socket::Type) {
		if (ptr) *reinterpret_cast<int *>(ptr) = _inputFile;
		return me;
	}
	if (type == to_pointer_id(metatype::Type)) {
		if (ptr) *static_cast<const metatype **>(ptr) = this;
		return output::Type;
	}
	if (type == to_pointer_id(output::Type)) {
		if (ptr) *static_cast<const output **>(ptr) = this;
		return me;
	}
	return BadType;
}
// object interface
int io::stream::property(struct property *pr) const
{
	int me = mpt_input_typeid();
	
	if (me < 0) {
		me = output::Type;
	}
	if (!pr) {
		return me;
	}
	const char *name;
	intptr_t pos;
	
	if (!(name = pr->name)) {
		if (((pos = (intptr_t) pr->desc) < 0)) {
			return BadValue;
		}
	}
	else {
		// get stream interface types
		if (!*name) {
			static const uint8_t fmt[] = { output::Type, 0 };
			pr->name = "stream";
			pr->desc = "interfaces to stream data";
			pr->val.fmt = fmt;
			pr->val.ptr = 0;
			return _srm ? mpt_stream_flags(&_srm->_info) : 0;
		}
	}
	intptr_t id = 0;
	if (name ? !strcasecmp(name, "idlen") : (pos == id++)) {
		static const uint8_t fmt[] = "y";
		pr->name = "idlen";
		pr->desc = "message id length";
		pr->val.fmt = fmt;
		pr->val.ptr = &_idlen;
		return id;
	}
	return BadArgument;
}
int io::stream::set_property(const char *pr, const metatype *src)
{
	if (!pr) {
		if (!_srm) {
			return BadOperation;
		}
		int ret = mpt_stream_setter(_srm, src);
		if (ret >= 0) {
			_ctx.set_instance(0);
		}
		return ret;
	}
	if (strcasecmp(pr, "idlen")) {
		if (_inputFile >= 0) {
			return BadOperation;
		}
		uint8_t l;
		
		if (!src) {
			l = 0;
		} else {
			int ret = src->conv('y', &l);
			if (ret < 0) {
				return ret;
			}
			if (l > std::numeric_limits<__decltype(_idlen)>::max() / 2 + 1) {
				return BadValue;
			}
		}
		_idlen = l;
		return 0;
	}
	return BadArgument;
}
bool io::stream::open(const char *dest, const char *type)
{
	if (_inputFile >= 0) {
		return false;
	}
	if (!_srm) _srm = new ::mpt::stream;
	if (mpt_stream_open(_srm, dest, type) < 0) {
		return false;
	}
	_inputFile = _mpt_stream_fread(&_srm->_info);
	return true;
}
bool io::stream::open(void *base, size_t len, int mode)
{
	if (_inputFile >= 0) {
		return false;
	}
	if (len && !base) {
		return false;
	}
	struct iovec data = { base, len };
	int ret;
	if (!_srm) _srm = new ::mpt::stream;
	switch (mode) {
		case ::mpt::stream::Read:
			ret = mpt_stream_memory(_srm, &data, 0);
			break;
		case ::mpt::stream::Write:
			ret = mpt_stream_memory(_srm, 0, &data);
			break;
		default:
			return false;
	}
	if (ret < 0) {
		return false;
	}
	return true;
}
// IODevice interface
ssize_t io::stream::write(size_t len, const void *data, size_t part)
{
	if (!_srm) {
		return BadArgument;
	}
	return mpt_stream_write(_srm, len, data, part);
}
ssize_t io::stream::read(size_t len, void *data, size_t part)
{
	if (!_srm) {
		return BadArgument;
	}
	return mpt_stream_read(_srm, len, data, part);
}
int64_t io::stream::pos()
{
	if (!_srm) {
		return BadArgument;
	}
	return mpt_stream_seek(_srm, 0, SEEK_CUR);
}
bool io::stream::seek(int64_t pos)
{
	if (!_srm || mpt_stream_seek(_srm, pos, SEEK_SET) < 0) {
		return false;
	}
	return true;
}
// input interface
int io::stream::next(int what)
{
	if (!_srm) {
		return BadArgument;
	}
	int ret = mpt_stream_poll(_srm, what, 0);
	if (ret < 0) {
		_inputFile = -1;
	}
	return ret;
}
class io::stream::dispatch
{
public:
	dispatch(stream &s, event_handler_t c, void *a) : srm(s), cmd(c), arg(a)
	{ }
	int process(const struct message *msg)
	{
		static const char _func[] = "mpt::io::stream::dispatch";
		struct event ev;
		int ret;
		
		if (!msg || !(ret = srm._idlen)) {
			ev.msg = msg;
			return cmd(arg, &ev);
		}
		uint8_t id[__UINT8_MAX__];
		uint8_t idlen = ret;
		
		struct message tmp = *msg;
		if (tmp.read(idlen, id) < idlen) {
			error(_func, "%s", MPT_tr("message id incomplete"));
		}
		if (id[0] & 0x80) {
			command *ans;
			uint64_t rid;
			id[0] &= 0x7f;
			if ((ret = mpt_message_buf2id(id, idlen, &rid)) < 0 || ret > (int) sizeof(ans->id)) {
				error(_func, "%s", MPT_tr("bad reply id"));
				return BadValue;
			}
			if ((ans = srm._wait.get(rid))) {
				int ret = ans->cmd(ans->arg, &tmp);
				ans->cmd = 0;
				return ret;
			}
			error(_func, "%s (id = %08" PRIx64 ")", MPT_tr("unknown reply id"), rid);
			return BadValue;
		}
		reply_data *rd = 0;
		reply_context *rc = 0;
		for (uint8_t i = 0; i < idlen; ++i) {
			if (!id[i]) {
				continue;
			}
			metatype *ctx;
			if (!(ctx = srm._ctx.instance())) {
				warning(_func, "%s", MPT_tr("no reply context"));
				break;
			}
			if (!(rd = ctx->cast<reply_data>())) {
				break;
			}
			if (!rd->set(idlen, id)) {
				error(_func, "%s", MPT_tr("reply context unusable"));
				return BadOperation;
			}
			rc = ctx->cast<reply_context>();
			break;
		}
		ev.reply = rc;
		ev.msg = &tmp;
		ret = cmd(arg, &ev);
		if (rc && rd && rd->active()) {
			struct msgtype mt(msgtype::Answer, ret);
			struct message msg(&mt, sizeof(mt));
			rc->reply(&msg);
		}
		return ret;
	}
protected:
	io::stream &srm;
	event_handler_t cmd;
	void *arg;
};
static int stream_dispatch(void *ptr, const struct message *msg)
{
	class io::stream::dispatch *sd = reinterpret_cast<class io::stream::dispatch *>(ptr);
	return sd->process(msg);
}
int io::stream::dispatch(event_handler_t cmd, void *arg)
{
	if (!_srm) {
		return BadArgument;
	}
	if (!_srm->_rd.pending_message()
	 && !_srm->_rd.advance()) {
		if (_mpt_stream_fread(&_srm->_info) < 0) {
			return BadArgument;
		}
		return 0;
	}
	class dispatch sd(*this, cmd, arg);
	return mpt_stream_dispatch(_srm, stream_dispatch, &sd);
}

ssize_t io::stream::push(size_t len, const void *src)
{
	if (!_srm) {
		return BadArgument;
	}
	ssize_t curr;
	if (_idlen && !(_srm->flags() & ::mpt::stream::MesgActive)) {
		uint8_t id[__UINT8_MAX__];
		mpt_message_id2buf(_cid, id, _idlen);
		if (mpt_stream_push(_srm, _idlen, id) < _idlen) {
			push(1, 0);
		}
	}
	if ((curr = mpt_stream_push(_srm, len, src)) < 0) {
		return curr;
	}
	if (!len) {
		_cid = 0;
	}
	else if (!src && _cid) {
		command *c;
		if ((c = _wait.get(_cid))) {
			c->cmd(c->arg, 0);
		}
	}
	return curr;
}
int io::stream::sync(int timeout)
{
	if (!_srm) {
		return BadArgument;
	}
	if (mpt_stream_flush(_srm) < 0) {
		return BadOperation;
	}
	if (!_idlen) {
		return true;
	}
	return mpt_stream_sync(_srm, _idlen, &_wait, timeout);
}

int io::stream::await(int (*rctl)(void *, const struct message *), void *rpar)
{
	if (!_idlen) {
		return BadArgument;
	}
	struct command *cmd;
	
	if (mpt_stream_flags(&_srm->_info) & _srm->MesgActive) {
		return message::InProgress;
	}
	if (!rctl) {
		return 0;
	}
	if (!(cmd = _wait.reserve(_idlen))) {
		return BadOperation;
	}
	_cid = cmd->id;
	cmd->cmd = (int (*)(void *, void *)) rctl;
	cmd->arg = rpar;
	return 1;
}

int io::stream::getchar()
{
	if (!_srm || _srm->_rd.encoded()) {
		return BadArgument;
	}
	if (_srm->_rd.len) {
		uint8_t *base = (uint8_t *) _srm->_rd.base;
		if (_srm->_rd.off >= _srm->_rd.max) {
			_srm->_rd.off = 0;
		}
		--_srm->_rd.len;
		return base[_srm->_rd.off++];
	}
	return io::interface::getchar();
}

void io::stream::close()
{
	if (_srm) {
		mpt_stream_close(_srm);
	}
}

__MPT_NAMESPACE_END
