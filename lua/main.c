#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "unistd.h"

int luaopen_mpt(lua_State *);

const char *stdRead(lua_State *L, void *data, size_t *size)
{
	ssize_t len = read(0, data, 256);
	(void) L;
	if (len < 0) return 0;
	*size = len;
	return data;
}

int main(int argc, char *argv[])
{
	const char *err;
	int i, ret;
	
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
#if LUA_VERSION_NUM < 502
	if ((ret = luaL_dostring(L, "mpt = require('mpt')"))) {
		fputs("mpt load error\n", stderr);
	}
#else
	luaL_requiref(L, "mpt", luaopen_mpt, 1);
#endif
	/*
	if ((s = luaL_dostring(L, "print('# ', mpt)"))) {
		fputs("print error\n", stderr);
	}
	*/
	ret = 0;
	
	if (luaL_dofile(L, "mathbox.lua")) {
		if ((err = lua_tostring(L, -1))) {
			fputs(err, stderr);
			fputc('\n', stderr);
		}
		return 2;
	}
	
	if (argc < 2) {
		char buf[256];
#if LUA_VERSION_NUM < 502
		if ((lua_load(L, stdRead, buf, "<stdin>")
#else
		if ((lua_load(L, stdRead, buf, "<stdin>", 0)
#endif
		     || lua_pcall(L, 0, LUA_MULTRET, 0))
		    && (err = lua_tostring(L, -1))) {
			fputs(err, stderr);
			fputc('\n', stderr);
		};
	}
	else for (i = 1; i < argc; ++i) {
		if (luaL_dofile(L, argv[i])) {
			if ((err = lua_tostring(L, -1))) {
				fputs(err, stderr);
				fputc('\n', stderr);
			}
			ret = 1;
			break;
		}
	}
	lua_close(L);
	
	return ret;
}
