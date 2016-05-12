
#include <stdlib.h>
#include <stdio.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(parse.h)

static int save(void *ctx, const MPT_STRUCT(path) *porg, int last, int curr)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	const char *base;
	int len, off;
	
	(void) last;
	(void) curr;
	
	if (!porg) {
		fputs("finished\n", stderr);
		return 0;
	}
	if (!porg->len) {
		if ((base = porg->base)) {
			fprintf(stderr, "%s %s\n", "parse", base);
		}
		return 0;
	}
	if ((curr & 0x3) == MPT_ENUM(ParseSectEnd)) {
		return 0;
	}
	fputc(' ', ctx);
	fputc(' ', ctx);
	
	p = *porg;
	base = p.base;
	off = p.off;
	while ((len = mpt_path_next(&p)) >= 0) {
		if (off) fputc('.', stdout);
		if (len) fwrite(base, len, 1, ctx);
		off = p.off;
		base = p.base + off;
	}
	if ((curr & 0x3) == MPT_ENUM(ParseOption)) {
		fputs(" = ", ctx);
	}
	if ((base = mpt_path_data(&p))) {
		fputs(base, ctx);
		fputc('\n', ctx);
		return 1;
	}
	fputc('\n', ctx);
	return 0;
}

extern int main(int argc, char *argv[])
{
	const char *root;
	int ret;
	
	if (argc > 1) root = argv[1];
	
	ret = mpt_config_load(save, stdout, mpt_log_default(0), root);
	
	if (ret < 0) {
		return 1;
	}
	return 0;
}
