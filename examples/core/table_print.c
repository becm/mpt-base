/*!
 * MPT base examples
 *   output node content
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

#include MPT_INCLUDE(meta.h)
#include MPT_INCLUDE(collection.h)
#include MPT_INCLUDE(convert.h)

struct print_context
{
	struct print_context *parent;
	FILE *out;
};

extern int table_print_level(void *cptr, const MPT_STRUCT(identifier) *id, MPT_INTERFACE(convertable) *conv, const MPT_INTERFACE(collection) *sub)
{
	
	const struct print_context *ctx = cptr, *parent;
	FILE *file = ctx->out;
	
	/* print leading dots to indicate element depth */
	while ((parent = ctx->parent)) {
		fputc('.', file);
		ctx = parent;
	}
	
	/* current identifier and value */
	if (id) {
		const char *str = mpt_identifier_data(id);
		fputs(str, file);
	}
	if (conv) {
		const char *str = 0;
		fputc('=', file);
		if (conv->_vptr->convert(conv, 's', &str) >= 0 && str) {
			fputs(str, file);
		}
	}
	fputs(mpt_newline_string(0), file);
	
	/* process subtree with increased depth */
	if (sub) {
		struct print_context next = { cptr, file };
		int ret = sub->_vptr->each(sub, table_print_level, &next);
		
		if (ret < 0) {
			return ret;
		}
	}
	
	return 0;
}

extern int table_print(void *fd, MPT_INTERFACE(convertable) *val, const MPT_INTERFACE(collection) *col)
{
	struct print_context ctx = { 0, fd };
	(void) val;
	return col ? col->_vptr->each(col, table_print_level, &ctx) : 0;
}
