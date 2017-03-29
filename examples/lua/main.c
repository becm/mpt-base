#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <stdlib.h>
#include <unistd.h>

int luaopen_mpt(lua_State *);

const char *stdRead(lua_State *L, void *data, size_t *size)
{
	ssize_t len = read(0, data, 256);
	(void) L;
	if (len < 0) return 0;
	*size = len;
	return data;
}

static int loadModule(lua_State *L, const char *mod)
{
	static const char *prefix = 0;
	
	if (!prefix && !(prefix = getenv("MPT_PREFIX"))) {
		return -1;
	}
	else {
		int len;
		char buf[256];
#if LUA_VERSION_NUM < 502
		const char *version = LUA_VERSION;
		snprintf(buf, sizeof(buf), "%s/%s/%s/%s.%s",
		         prefix, "share/lua", version+4, mod, "lua");
#else
		snprintf(buf, sizeof(buf), "%s/%s/%s.%s",
		         prefix, "share/lua/" LUA_VERSION_MAJOR "." LUA_VERSION_MINOR, mod, "lua");
#endif
		len = lua_gettop(L);
		if (luaL_dofile(L, buf)) {
			const char *err;
			if ((err = lua_tostring(L, -1))) {
				fputs(err, stderr);
				fputc('\n', stderr);
			}
			lua_pop(L, 1);
			return -1;
		}
		return lua_gettop(L) - len;
	}
}

static int setGlobal(lua_State *L)
{
#if LUA_VERSION_NUM < 502
	lua_replace(L, LUA_GLOBALSINDEX);
#else
	lua_pushglobaltable(L);  /* get environment from registry */
	lua_newtable(L);  /* metatable for new globals */
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -4);  /* use table on stack */
	lua_settable(L, -3);
	lua_setmetatable(L, -2);  /* replace global metatable */
	lua_pop(L, 2);  /* remove global table and new environment from stack */
#endif
	return 0;
}

int main(int argc, char *argv[])
{
	const char *str;
	int i, ret;
	
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	/* load MPT module */
	if ((ret = loadModule(L, "mpt")) != 1) {
		fputs("failed to load mpt", stderr);
		fputc('\n', stderr);
		lua_close(L);
		return 2;
	}
	/* get global math for table */
	lua_getglobal(L, "math");
	
	/* get mpt.push for math environment */
#if LUA_VERSION_NUM < 503
	lua_getfield(L, -2, "push");
	if (!lua_isfunction(L, -1))
#else
	if ((ret = lua_getfield(L, -2, "push")) != LUA_TFUNCTION)
#endif
	{
		fputs("bad push function type", stderr);
		fputc('\n', stderr);
		lua_close(L);
		return 2;
	}
	lua_setfield(L, -2, "push"); /* removes push from stack */
	
	/* set math table as environment */
	if (setGlobal(L) < 0) {
		fputs("failed to apply mathbox", stderr);
		fputc('\n', stderr);
		lua_close(L);
		return 2;
	}
	lua_pop(L, 1); /* remove `mpt` module from stack */
	ret = 0;
	if (argc < 2) {
		char buf[256];
#if LUA_VERSION_NUM < 502
		if ((lua_load(L, stdRead, buf, "<stdin>")
#else
		if ((lua_load(L, stdRead, buf, "<stdin>", "t")
#endif
		     || lua_pcall(L, 0, LUA_MULTRET, 0))
		    && (str = lua_tostring(L, -1))) {
			fputs(str, stderr);
			fputc('\n', stderr);
		};
	}
	else for (i = 1; i < argc; ++i) {
		if (luaL_dofile(L, argv[i])) {
			if ((str = lua_tostring(L, -1))) {
				fputs(str, stderr);
				fputc('\n', stderr);
			}
			ret = 1;
			break;
		}
	}
	lua_close(L);
	
	return ret;
}
