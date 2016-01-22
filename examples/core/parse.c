/* Test MPT Parsing Functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#include <mpt/array.h>
#include <mpt/config.h>
#include <mpt/parse.h>
#include <mpt/stream.h>

int main(int argc, char *argv[])
{
	struct mpt_path path = MPT_PATH_INIT;
	struct mpt_parse parse = MPT_PARSE_INIT;
	struct mpt_parsefmt fmt;
	MPT_TYPE(ParserFcn) next;
	int type;
	
	mtrace();
	
	(void) mpt_config_environ(0, "mpt_*", '_', 0);
	
	type = mpt_parse_format(&fmt, argv[1]);
	
	if (!(next = mpt_parse_next_fcn(type))) {
		return 3;
	}
	/* explicit flags for allowed names */
	if (argc > 2 && mpt_parse_accept(&parse.name, argv[2]) < 0) {
		return 4;
	}
	parse.src.getc = (int (*)(void *)) mpt_getchar_stdio;
	parse.src.arg  = stdin;
	
	path.flags = MPT_ENUM(PathSepBinary);
	
	while ((type = next(&fmt, &parse, &path)) > 0) {
		/* skip section end events */
		if (type & (MPT_ENUM(ParseSection) | MPT_ENUM(ParseData))) {
			mpt_path_fputs(&path, stdout, " = ", "/");
			fputc('\n', stdout);
		}
		/* clear path element on section end or option */
		if (type & MPT_ENUM(ParseSectEnd)) {
			mpt_path_del(&path);
		} else {
			mpt_path_invalidate(&path);
		}
		parse.prev = parse.curr;
	}
	if (type) {
		fprintf(stderr, "parse error %d, line %lu\n", type, parse.src.line);
	}
	mpt_path_fini(&path);
	
	return -type;
}

