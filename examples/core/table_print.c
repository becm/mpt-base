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

#include MPT_INCLUDE(node.h)
#include MPT_INCLUDE(meta.h)
#include MPT_INCLUDE(convert.h)

extern int table_print(struct mpt_node *node, void *file, size_t depth)
{
	struct mpt_metatype *meta;
	const char *id = mpt_node_ident(node);
	
	while (depth--) fputc('.', file);
	if (id) fputs(id, file);
	
	if ((meta = node->_meta)) {
		fputc('=', file);
		if (meta->_vptr->conv(meta, 's', &id) >= 0 && id) {
			fputs(id, file);
		}
	}
	fputs(mpt_newline_string(0), file);
	return 0;
}
