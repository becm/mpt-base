/*!
 * readline loader and fallback implemetation
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dlfcn.h>

#include "client.h"

static char *_readline(const char *prompt)
{
	char	*buf = 0, *curr = 0, *end;
	size_t	len = 64;
	
	fputs(prompt, stdout);
	
	while (1) {
		size_t	clen = buf+len-curr;
		
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
			*end = 0;
			return buf;
		}
		len += 64;
		curr += 63;
	}
}

static const char * const backends[] = {
	"libreadline.so.6",
	"libeditline.so.0",
	"libreadline.so.5",
	0
};

static void *lib_glob;

static void close_readline(void)
{
	if (lib_glob) dlclose(lib_glob);
}

extern char *mpt_readline(const char *prompt)
{
	static union {
		char *(*fcn)(const char *);
		void *addr;
	} r, h;
	const char * const *be;
	char *ret;
	
	if (r.fcn) {
		ret = r.fcn(prompt);
		if (h.fcn && ret && *ret && !isspace(*ret)) {
			h.fcn(ret);
		}
		return ret;
	}
	
	if (getenv("MALLOC_TRACE")) {
		fputs("disabled readline support for memory tracing\n", stderr);
		return (r.fcn = _readline)(prompt);
	}
	
	h.addr = dlsym(lib_glob, "add_history");
	r.addr = dlsym(lib_glob, "readline");
	
	be = backends;
	
	while (!r.addr) {
		if (lib_glob) dlclose(lib_glob);
		
		if (!(lib_glob = dlopen(*be, RTLD_LAZY))) {
			if (!*(++be)) break;
			continue;
		}
		h.addr = dlsym(lib_glob, "add_history");
		r.addr = dlsym(lib_glob, "readline");
	}
	/* fallback implementation */
	if (!r.addr) {
		h.addr = 0;
		return (r.fcn = _readline)(prompt);
	}
	atexit(close_readline);
	
	ret = r.fcn(prompt);
	if (h.fcn && ret && *ret && !isspace(*ret)) {
		h.fcn(ret);
	}
	return ret;
}
