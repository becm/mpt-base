#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>


#include <sys/uio.h>

#include <sys/wait.h>

#include "queue.h"
#include "message.h"
#include "convert.h"

#include "output.h"

#include "../mptio/stream.h"

struct luaStream {
	MPT_STRUCT(stream) srm;
	pid_t pid;
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
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
	lua_pushfstring(L, "%s (%p)", streamClassString, s);
	return 1;
}
static int streamWait(struct luaStream *s)
{
	pid_t id;
	int end;
	if ((id = s->pid)
	    && (id == waitpid(s->pid, &end, WNOHANG))
	    && (WIFEXITED(end)
	        || WIFSIGNALED(end))) {
		s->pid = 0;
		return 1;
	}
	return 0;
}
static int streamDel(lua_State *L)
{
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
	streamWait(s);
	mpt_stream_close(&s->srm);
	return 0;
}
static int streamRead(lua_State *L)
{
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
	size_t len;
	void *base;
	int timeout = -1;
	int flg;
	luaL_Buffer b;
	
	streamWait(s);
	
	flg = mpt_stream_flags(&s->srm._info);
	
	if (MPT_stream_writable(flg)
	    && !(flg & MPT_ENUM(StreamRdWr))) {
		return 0;
	}
	
	if (lua_isnumber(L, 2)) {
		timeout = lua_tonumber(L, 2);
	}
	if (mpt_stream_poll(&s->srm, POLLIN, timeout) < 0) {
		return 0;
	}
	if (s->srm._rd.data.len == s->srm._rd.data.max) {
		while ((timeout = mpt_stream_poll(&s->srm, POLLIN, 0)) > 0
		       && timeout & POLLIN);
	}
	luaL_buffinit(L, &b);
	base = mpt_queue_data(&s->srm._rd.data, &len);
	if (len) luaL_addlstring(&b, base, len);
	len = s->srm._rd.data.len - len;
	if (len) luaL_addlstring(&b, s->srm._rd.data.base, len);
	
	mpt_queue_crop(&s->srm._rd.data, 0, s->srm._rd.data.len);
	
	luaL_pushresult(&b);
	return 1;
}
static int streamWrite(lua_State *L)
{
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
	int i, max;
	
	streamWait(s);
	
	if (s->srm._wd._enc) {
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
		if (!(l = mpt_stream_write(&s->srm, l, d, 1))) {
			break;
		}
	}
	lua_pushinteger(L, i - 2);
	return 1;
}
static int streamPush(lua_State *L)
{
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
	int i, flg, len;
	size_t old;
	
	streamWait(s);
	
	if (!s->srm._wd._enc) {
		lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("push to raw stream"));
		return lua_error(L);
	}
	old = s->srm._wd.data.len;
	
	/* signal no further data */
	if ((len = lua_gettop(L)) == 1) {
		if (mpt_stream_push(&s->srm, 0, 0) < 0) {
			lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to terminate data"));
			return lua_error(L);
		}
		s->mt.cmd = 0;
		s->mt.arg = 0;
		old = s->srm._wd.data.len - old;
		lua_pushinteger(L, old);
		return 1;
	}
	flg = mpt_stream_flags(&s->srm._info);
	
	i = 2;
	if (!(flg & MPT_ENUM(StreamMesgAct))) {
		s->mt.cmd = 0;
		s->mt.arg = 0;
		if (flg & MPT_ENUM(StreamRdWr)) {
			uint16_t mid = 0;
			
			if (mpt_stream_push(&s->srm, sizeof(mid), &mid) < 0) {
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to terminate data"));
				return lua_error(L);
			}
		}
		/* header without string separation */
		if (lua_type(L, i) == LUA_TSTRING) {
			const char *d;
			size_t l;
			
			if (!(d = lua_tolstring(L, i++, &l))) {
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("bad header data"));
				return lua_error(L);
			}
			if (l) {
				s->mt.cmd = d[0];
			}
			if (l > 1) {
				s->mt.arg = d[1];
			}
			if (mpt_stream_push(&s->srm, l, d) < 0) {
				mpt_stream_push(&s->srm, 1, 0);
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to push header data"));
				return lua_error(L);
			}
		}
	}
	while (i <= len) {
		int t = lua_type(L, i);
		
		/* write raw number data */
		if (t == LUA_TNUMBER) {
			double v = lua_tonumber(L, i);
			if (mpt_stream_push(&s->srm, sizeof(v), &v) < 0) {
				mpt_stream_push(&s->srm, 1, 0);
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to push data"));
				return lua_error(L);
			}
			++i;
			continue;
		}
		/* simple continous command */
		if (t == LUA_TSTRING) {
			const char *d;
			size_t l;
			
			if ((s->mt.cmd == MPT_ENUM(MessageCommand))
			    && mpt_stream_push(&s->srm, 1, &s->mt.arg) < 0) {
				mpt_stream_push(&s->srm, 1, 0);
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to push separator"));
				return lua_error(L);
			}
			/* include separation */
			if ((d = lua_tolstring(L, i, &l))
			    && ((mpt_stream_push(&s->srm, l, d) < 0))) {
				mpt_stream_push(&s->srm, 1, 0);
				lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("unable to push data"));
				return lua_error(L);
			}
			++i;
			continue;
		}
		mpt_stream_push(&s->srm, 1, 0);
		lua_pushfstring(L, "%s\t%s", streamClassString, MPT_tr("invalid data type"));
		return lua_error(L);
	}
	old = s->srm._wd.data.len - old;
	lua_pushinteger(L, old);
	return 1;
}
static int streamFlush(lua_State *L)
{
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
	streamWait(s);
	mpt_stream_flush(&s->srm);
	return 0;
}
static int streamConnect(lua_State *L)
{
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	MPT_STRUCT(fdmode) mode;
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
	const char *dest = luaL_checkstring(L, 2);
	const char *type = lua_isstring(L, 3) ? lua_tostring(L, 3) : 0;
	int flg;
	
	if (type) {
		if ((flg = mpt_mode_parse(&mode, type)) < 0) {
			type = 0;
		}
	}
	if ((flg = mpt_connect(&sock, dest, type ? &mode : 0)) < 0) {
		return 0;
	}
	if ((flg = mpt_stream_sockflags(flg)) < 0) {
		(void) close(sock._id);
		return 0;
	}
	if (mpt_stream_dopen(&s->srm, &sock, type ? mode.stream : flg) < 0) {
		close(sock._id);
		return 0;
	}
	lua_pushvalue(L, 1);
	return 1;
}
static int streamIndex(lua_State *L)
{
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
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
		if (s->srm._wd._enc == mpt_encode_cobs) {
			lua_pushcfunction(L, bufferCobs);
		}
		else if (s->srm._wd._enc || s->srm._rd._dec) {
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
	struct luaStream *s = luaL_checkudata(L, 1, streamClassString);
	const char *id = luaL_checkstring(L, 2);
	
	if (!strcmp(id, "encoding")) {
		MPT_TYPE(DataEncoder) enc;
		MPT_TYPE(DataDecoder) dec;
		int type = -1;
		
		if (s->srm._rd.data.base || s->srm._wd.data.base) {
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
		s->srm._wd._enc = enc;
		s->srm._rd._dec = dec;
	}
	
	return 0;
}

static struct luaStream *createStream(lua_State *L)
{
	static const MPT_STRUCT(stream) def = MPT_STREAM_INIT;
	struct luaStream *s;
	
	if (!(s = lua_newuserdata(L, sizeof(*s)))) {
		return 0;
	}
	s->srm = def;
	s->pid = 0;
	
	/* use existing metatable */
	if (!luaL_newmetatable(L, streamClassString)) {
		lua_setmetatable(L, -2);
		return s;
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
	
	return s;
}
static int streamCreate(lua_State *L)
{
	return createStream(L) ? 1 : 0;
}
static int streamOpen(lua_State *L)
{
	struct luaStream *s;
	const char *dest = lua_tostring(L, 1);
	const char *mode = lua_tostring(L, 2);
	
	if (!(s = createStream(L))) {
		return 0;
	}
	if (!dest) {
		return 0;
	}
	if (!*dest) {
		_mpt_stream_setfile(&s->srm._info, 0, 1);
	}
	else if (mpt_stream_open(&s->srm, dest, mode) < 0) {
		return 0;
	}
	return 1;
}
static int streamPipe(lua_State *L)
{
	struct luaStream *s;
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
	
	if (!(s = createStream(L))) {
		free(args);
		return 0;
	}
	else if ((s->pid = mpt_stream_pipe(&s->srm._info, args[0], args)) <= 0) {
		free(args);
		return 0;
	}
	free(args);
	mpt_stream_setmode(&s->srm, MPT_ENUM(StreamBuffer));
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
