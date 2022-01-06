/*!
 * MPT base examples
 *   file parser modes and formats
 */

#include <stdio.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(parse.h)

int main(int argc, char *argv[])
{
	struct mpt_path path = MPT_PATH_INIT;
	struct mpt_parser_context parse = MPT_PARSER_INIT;
	struct mpt_parser_format fmt;
	MPT_TYPE(input_parser) next;
	int type;
	
	mtrace();
	
	(void) mpt_config_environ(0, "mpt_*", '_', 0);
	next = (MPT_TYPE(input_parser)) mpt_parse_format_pre;
	
	parse.src.getc = (int (*)(void *)) mpt_getchar_stdio;
	parse.src.arg = stdin;
	
	if (argc > 1) {
		if  (!(parse.src.arg = fopen(argv[1], "r"))) {
			fputs("missing file name\n", stderr);
			return 2;
		}
	}
	if (argc > 2) {
		type = mpt_parse_format(&fmt, argv[2]);
		
		if (!(next = mpt_parse_next_fcn(type))) {
			fputs("bad format type\n", stderr);
			return 3;
		}
	}
	/* explicit flags for allowed names */
	if (argc > 3 && mpt_parse_accept(&parse.name, argv[3]) < 0) {
		return 4;
	}
	
	path.flags = MPT_PATHFLAG(SepBinary);
	
	while ((type = next(&fmt, &parse, &path)) > 0) {
		/* skip section end events */
		if (type & (MPT_ENUM(ParseSection) | MPT_ENUM(ParseData))) {
			mpt_path_fputs(&path, stdout, "/");
			if (type & MPT_ENUM(ParseData)) {
				fputs(" = ", stdout);
				fwrite(path.base + path.off + path.len, parse.valid, 1, stdout);
			}
			fputc('\n', stdout);
		}
		/* clear path element on section end or option */
		if (type & MPT_ENUM(ParseSectEnd)) {
			mpt_path_del(&path);
		} else {
			mpt_path_invalidate(&path);
		}
		parse.prev = parse.curr;
		parse.valid = 0;
	}
	if (type) {
		fprintf(stderr, "parse error %d, line %lu\n", type, parse.src.line);
	}
	mpt_path_fini(&path);
	
	return -type;
}

