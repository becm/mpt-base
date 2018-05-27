/*!
 * readline loader and fallback implemetation
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dlfcn.h>

#include "loader.h"

static char *_readline(const char *prompt)
{
	char *buf = 0, *curr = 0, *end;
	size_t len = 64;
	
	fputs(prompt, stdout);
	
	while (1) {
		size_t clen = (buf - curr) + len;
		
		if (!(curr = realloc(buf, len))) {
			free(buf);
			return 0;
		}
		curr = (buf = curr) + len - clen;
		
		if (!(end = fgets(curr, clen, stdin))) {
			free(buf);
			return 0;
		}
		if (!*end || (end = memchr(end, '\n', clen))) {
			if (end == buf) {
				static const char empty = 0;
				free(buf);
				return (char *) &empty;
			}
			*end = 0;
			return buf;
		}
		len += 64;
		curr += 63;
	}
}

static void *lib_glob;

static void close_readline(void)
{
	dlclose(lib_glob);
}
static void *open_backend(void)
{
	static const char * const backends[] = {
		"libreadline.so.7",
		"libeditline.so.0",
		"libreadline.so.6",
		"libreadline.so.5",
		0
	};
	const char * const *be = backends;
	const char *lib;
	
	while ((lib = *be++)) {
		void *addr;
		if (!(lib_glob = dlopen(lib, RTLD_LAZY))) {
			continue;
		}
		addr = dlsym(lib_glob, "readline");
		
		if (addr) {
			atexit(close_readline);
			return addr;
		}
		dlclose(lib_glob);
	}
	return 0;
}

extern char *mpt_readline(const char *prompt)
{
	static union {
		char *(*fcn)(const char *);
		void *addr;
	} r = { 0 }, h;
	
	/* backend already selected */
	if (r.fcn) {
		char *ret = r.fcn(prompt);
		if (h.fcn && ret && *ret && !isspace(*ret)) {
			h.fcn(ret);
		}
		return ret;
	}
	/* no 3rd party allocations */
	if (getenv("MALLOC_TRACE")) {
		fputs("disabled readline support for memory tracing\n", stderr);
		h.addr = 0;
		return (r.fcn = _readline)(prompt);
	}
	/* use linked implementation */
	r.addr = dlsym(lib_glob, "readline");
	
	/* alternate shared lib implementation */
	if (r.addr || (r.addr = open_backend())) {
		h.addr = dlsym(lib_glob, "add_history");
	}
	/* fallback implementation */
	else {
		r.fcn = _readline;
		h.addr = 0;
	}
	/* (re)call with defined backend */
	return mpt_readline(prompt);
}
