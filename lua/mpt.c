#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>


#include <sys/uio.h>

#include "queue.h"
#include "message.h"
#include "convert.h"

#include "../mptplot/output.h"

#include "../mptio/stream.h"

struct luaStream {
	MPT_STRUCT(stream) srm;
	MPT_STRUCT(msgtype) mt;
};

static int bufferCobs(lua_State *L)
{
	MPT_STRUCT(codestate) s;
	struct iovec from, to;
	ssize_t take;
	luaL_Buffer b;
	
	if (!(from.iov_base = (void *) lua_tolstring(L, 1, &from.iov_len))) {
		return 0;
	}
	luaL_buffinit(L, &b);
	to.iov_len = from.iov_len + (from.iov_len/255) + 4;
	
#if LUA_VERSION_NUM < 502
	if ((to.iov_len > LUAL_BUFFERSIZE)
	    || (!(to.iov_base = luaL_prepbuffer(&b)))) {
		return 0;
	}
#else
	if (!(to.iov_base = luaL_prepbuffsize(&b, to.iov_len))) {
		return 0;
	}
#endif
	memset(&s, 0, sizeof(s));
	if ((take = mpt_encode_cobs(&s, &to, &from)) < 0
	    || mpt_encode_cobs(&s, &to, 0) < 0
	    || take != (ssize_t) from.iov_len) {
		return 0;
	}
	take = s.done;
	mpt_encode_cobs(&s, 0, 0);
	luaL_addsize(&b, take);
	luaL_pushresult(&b);
	return 1;
}

static const char streamClassString[] = "mpt::stream\0";


static int streamString(lua_State *L)
{
	MPT_STRUCT(stream) *s = luaL_checkudata(L, 1, streamClassString);
	lua_pushfstring(L, "%s (%p)", streamClassString, s);
	return 1;
}
static int streamDel(lua_State *L)
{
	MPT_STRUCT(stream) *s = luaL_checkudata(L, 1, streamClassString);
	mpt_stream_close(s);
	return 0;
}
static int streamRead(lua_State *L)
{
	MPT_STRUCT(stream) *s = luaL_checkudata(L, 1, streamClassString);
	size_t len;
	void *base;
	int timeout = -1;
	luaL_Buffer b;
	
	if (lua_isnumber(L, 2)) {
		timeout = lua_tonumber(L, 2);
	}
	if (mpt_stream_poll(s, POLLIN, timeout) < 0) {
		return 0;
	}
	if (s->_rd.len == s->_rd.max) {
		while ((timeout = mpt_stream_poll(s, POLLIN, 0)) > 0
		       && timeout & POLLIN);
	}
	luaL_buffinit(L, &b);
	base = mpt_queue_data(&s->_rd, &len);
	if (len) luaL_addlstring(&b, base, len);
	len = s->_rd.len - len;
	if (len) luaL_addlstring(&b, s->_rd.base, len);
	
	mpt_queue_crop(&s->_rd, 0, s->_rd.len);
	
	luaL_pushresult(&b);
	return 1;
}
static int streamWrite(lua_State *L)
{
	MPT_STRUCT(stream) *s = luaL_checkudata(L, 1, streamClassString);
	int i, max;
	
	if (s->_enc.fcn) {
		lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("write to encoded stream"));
		return lua_error(L);
	}
	max = lua_gettop(L);
	i = 1;
	while (i++ < max) {
		const void *d;
		size_t l;
		
		if (!(d = lua_tolstring(L, i, &l))) {
			break;
		}
		if (!(l = mpt_stream_write(s, l, d, 1))) {
			break;
		}
	}
	lua_pushinteger(L, i - 2);
	return 1;
}
static int streamPush(lua_State *L)
{
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
	int i, t, len;
	size_t old;
	
	if (!s->srm._enc.fcn) {
		lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("push to raw stream"));
		return lua_error(L);
	}
	old = s->srm._wd.len;
	
	/* signal no further data */
	if ((len = lua_gettop(L)) == 1) {
		if (mpt_stream_push(&s->srm, 0, 0) < 0) {
			lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to terminate data"));
			return lua_error(L);
		}
		old = s->srm._wd.len - old;
		lua_pushinteger(L, old);
		return 1;
	}
	
	i = 2;
	t = lua_type(L, i);
	/* determine message type */
	if (!(mpt_stream_flags(&s->srm._info) & MPT_ENUM(StreamMesgAct))) {
		if (t == LUA_TNUMBER) {
			s->mt.cmd = MPT_ENUM(MessageValRaw);
			s->mt.arg = (int8_t) (MPT_ENUM(ByteOrderNative) | MPT_ENUM(ValuesFloat) | sizeof(double));
		}
		/* simple continous command */
		else if (t == LUA_TSTRING) {
			s->mt.cmd = MPT_ENUM(MessageCommand);
			s->mt.arg = ' ';
		}
		else {
			lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("invalid message type"));
			return lua_error(L);
		}
		++i;
	}
	if (s->mt.cmd == MPT_ENUM(MessageValRaw)
	    || s->mt.cmd == MPT_ENUM(MessageValFmt)
	    || s->mt.cmd == MPT_ENUM(MessageDest)) {
		/* check arguments */
		for (; i <= len; ++i) {
			if (lua_type(L, i) != LUA_TNUMBER) {
				lua_pushfstring(L, "%s\t%d (pos %d)", streamClassString, MPT_tr("invalid number data"), i);
				return lua_error(L);
			}
		}
		if (!(mpt_stream_flags(&s->srm._info) & MPT_ENUM(StreamMesgAct))) {
			size_t pre = 0;
			uint8_t hdr[sizeof(MPT_STRUCT(msgtype))+sizeof(MPT_STRUCT(laydest))+sizeof(MPT_STRUCT(msgworld))] = { 0 };
			
			switch (s->mt.cmd) {
			  case MPT_ENUM(MessageValRaw):
				hdr[0] = s->mt.cmd;
				hdr[3] = s->mt.arg;
				pre = 4;
				break;
			  case MPT_ENUM(MessageValFmt):
				hdr[0] = s->mt.cmd;
				hdr[1] = 1;
				hdr[2] = 1;
				hdr[3] = s->mt.arg;
				pre = 4;
				break;
			  case MPT_ENUM(MessageDest):
				hdr[0] = s->mt.cmd;
				hdr[3] = s->mt.arg;
				pre = sizeof(hdr);
			  default:;
			}
			if (pre && mpt_stream_push(&s->srm, pre, hdr) < 0) {
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to push data"));
				return lua_error(L);
			}
		}
		for (i = 2; i <= len; ++i) {
			double v = lua_tonumber(L, i);
			if (mpt_stream_push(&s->srm, sizeof(v), &v) < 0) {
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to push data"));
				return lua_error(L);
			}
		}
	}
	else if (s->mt.cmd == MPT_ENUM(MessageCommand)) {
		for (; i <= len; ++i) {
			if (lua_type(L, i) != LUA_TSTRING) {
				lua_pushfstring(L, "%s\t%s (pos %d)", streamClassString, MPT_tr("invalid string data"), i);
				return lua_error(L);
			}
		}
		if (!(mpt_stream_flags(&s->srm._info) & MPT_ENUM(StreamMesgAct))) {
			if (mpt_stream_push(&s->srm, sizeof(s->mt.cmd), &s->mt.cmd) < 0) {
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to push data"));
				return lua_error(L);
			}
		}
		for (i = 2; i <= len; ++i) {
			const char *d;
			size_t l;
			
			if (!(d = lua_tolstring(L, i, &l))) {
				continue;
			}
			/* include separation */
			if ((mpt_stream_push(&s->srm, 1, &s->mt.arg) < 0)
			    || (mpt_stream_push(&s->srm, l, d) < 0)) {
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to push data"));
				return lua_error(L);
			}
		}
	}
	old = s->srm._wd.len - old;
	lua_pushinteger(L, old);
	return 1;
}
static int streamFlush(lua_State *L)
{
	MPT_STRUCT(stream) *srm = luaL_checkudata(L, 1, streamClassString);
	mpt_stream_flush(srm);
	return 0;
}
static int streamConnect(lua_State *L)
{
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	MPT_STRUCT(fdmode) mode;
	MPT_STRUCT(stream) *srm = luaL_checkudata(L, 1, streamClassString);
	const char *dest = luaL_checkstring(L, 2);
	const char *type = lua_isstring(L, 3) ? lua_tostring(L, 3) : 0;
	int flg;
	
	if (type) {
		if ((flg = mpt_mode_parse(&mode, type)) < 0) {
			(void) mpt_mode_parse(&mode, 0);
		}
	}
	if ((flg = mpt_connect(&sock, dest, type ? &mode : 0)) < 0) {
		return 0;
	}
	if (!(flg & MPT_ENUM(SocketStream))) {
		(void) close(sock._id);
		return 0;
	}
	if (flg & MPT_ENUM(SocketWrite)) {
		if (flg & MPT_ENUM(SocketRead)) {
			flg = MPT_ENUM(StreamRdWr);
		} else {
			flg = MPT_ENUM(StreamWrite);
		}
	} else {
		flg = MPT_ENUM(StreamRead);
	}
	if (mpt_stream_dopen(srm, &sock, flg | MPT_ENUM(StreamBuffer)) < 0) {
		close(sock._id);
		return 0;
	}
	lua_pushvalue(L, 1);
	return 1;
}
static int streamIndex(lua_State *L)
{
	MPT_STRUCT(stream) *s = luaL_checkudata(L, 1, streamClassString);
	const char *id = luaL_checkstring(L, 2);
	
	if (!strcmp(id, "write")) {
		lua_pushcfunction(L, streamWrite);
		return 1;
	}
	if (!strcmp(id, "push")) {
		lua_pushcfunction(L, streamPush);
		return 1;
	}
	if (!strcmp(id, "read")) {
		lua_pushcfunction(L, streamRead);
		return 1;
	}
	if (!strcmp(id, "flush")) {
		lua_pushcfunction(L, streamFlush);
		return 1;
	}
	if (!strcmp(id, "connect")) {
		lua_pushcfunction(L, streamConnect);
		return 1;
	}
	if (!strcmp(id, "encoding")) {
		if (s->_enc.fcn == mpt_encode_cobs) {
			lua_pushcfunction(L, bufferCobs);
		}
		else if (s->_enc.fcn || s->_dec.fcn) {
			lua_pushboolean(L, 1);
		}
		else {
			lua_pushboolean(L, 0);
		}
		return 1;
	}
	return 0;
}
static int streamNewIndex(lua_State *L)
{
	MPT_STRUCT(stream) *s = luaL_checkudata(L, 1, streamClassString);
	const char *id = luaL_checkstring(L, 2);
	
	if (!strcmp(id, "encoding")) {
		MPT_TYPE(DataEncoder) enc;
		MPT_TYPE(DataDecoder) dec;
		int type = -1;
		
		if (s->_rd.base || s->_wd.base) {
			lua_pushstring(L, "stream is active");
			lua_error(L);
		}
		if (lua_isnil(L,3)) {
			type = 0;
		}
		else if (lua_isboolean(L,3)) {
			type = lua_toboolean(L,3) ? MPT_ENUM(EncodingCobs) : 0;
		}
		else if (lua_isnumber(L,3)) {
			type = lua_tonumber(L, 3);
		}
		else if (lua_isstring(L,3)) {
			type = mpt_encoding_value(id = lua_tostring(L, 3), -1);
		}
		/* no encoding */
		if (!type) {
			enc = 0;
			dec = 0;
		}
		/* asymetric encoding */
		else if (type == MPT_ENUM(EncodingCommand)) {
			enc = mpt_encode_string;
			dec = 0;
		}
		/* symetric encoding */
		else if ((enc = mpt_message_encoder(type))
		         && (dec = mpt_message_decoder(type))) {
			;
		}
		else {
			lua_pushstring(L, "invalid encoding");
			return lua_error(L);
		}
		s->_enc.fcn = enc;
		s->_dec.fcn = dec;
	}
	
	return 0;
}

static MPT_STRUCT(stream) *createStream(lua_State *L)
{
	static const MPT_STRUCT(stream) def = MPT_STREAM_INIT;
	struct luaStream *s;
	
	if (!(s = lua_newuserdata(L, sizeof(*s)))) {
		return 0;
	}
	s->srm = def;
	
	/* use existing metatable */
	if (!luaL_newmetatable(L, streamClassString)) {
		lua_setmetatable(L, -2);
		return &s->srm;
	}
	/* string conversion */
	lua_pushliteral(L, "__tostring");
	lua_pushcfunction(L, streamString);
	lua_settable(L, -3);
	/* garbage collection */
	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, streamDel);
	lua_settable(L, -3);
	
	/* index access for "member functions" */
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, streamIndex);
	lua_settable(L, -3);
	/* index access for "member functions" */
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, streamNewIndex);
	lua_settable(L, -3);
	
	lua_setmetatable(L, -2);
	
	return &s->srm;
}
static int streamCreate(lua_State *L)
{
	return createStream(L) ? 1 : 0;
}
static int streamOpen(lua_State *L)
{
	MPT_STRUCT(stream) *srm;
	const char *dest = lua_tostring(L, 1);
	const char *mode = lua_tostring(L, 2);
	
	if (!(srm = createStream(L))) {
		return 0;
	}
	if (!dest) {
		return 0;
	}
	if (!*dest) {
		_mpt_stream_setfile(&srm->_info, 0, 1);
	}
	else if (mpt_stream_open(srm, dest, mode) < 0) {
		return 0;
	}
	return 1;
}
static int streamPipe(lua_State *L)
{
	MPT_STRUCT(stream) *srm;
	char **args, *prog;
	int i, len = 0;
	
	if (!(prog = (char *) luaL_checkstring(L, 1))) {
		return 0;
	}
	len = lua_gettop(L);
	args = malloc((len + 1) * sizeof(*args));
	args[0] = prog;
	for (i = 1; i < len; ++i) {
		if (!(args[i] = (char *) lua_tostring(L, i+1))) {
			args[i] = "";
		}
	}
	args[i] = 0;
	
	if (!(srm = createStream(L))) {
		free(args);
		return 0;
	}
	else if (mpt_stream_pipe(&srm->_info, args[0], args) < 0) {
		free(args);
		return 0;
	}
	free(args);
	mpt_stream_setmode(srm, MPT_ENUM(StreamBuffer));
	return 1;
}
static const luaL_Reg mptInterface[] =
{
	{ "stream", streamCreate },
	{ "open",   streamOpen },
	{ "pipe",   streamPipe },
	{ "cobs",   bufferCobs },
	{ 0,       0 }
};

/* create MPT module */
int luaopen_mpt(lua_State *L)
{
lua_newtable(L);
#if LUA_VERSION_NUM < 502
luaL_register(L,0,mptInterface);
#else
luaL_setfuncs(L,mptInterface,0);
#endif

lua_pushliteral(L,"version");
lua_pushliteral(L,"1.0");
lua_settable(L,-3); /* Apply to module table */

return 1;
}
