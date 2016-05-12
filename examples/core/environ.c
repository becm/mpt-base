/*!
 * test MPT environment inclusion
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(node.h)
#include MPT_INCLUDE(config.h)

static int table_print(struct mpt_node *node, void *file, size_t depth)
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
	fputc('\n', file);
	return 0;
}

int main(int argc, char *argv[])
{
	mtrace();
	
	for (++argv; --argc; ++argv) {
		mpt_config_environ(0, *argv, '_', 0);
	}
	mpt_config_set(0, "ls.colors", 0, '.', 0);
	
	mpt_gnode_traverse(mpt_config_node(0), MPT_ENUM(TraversePreOrder) | MPT_ENUM(TraverseAll), table_print, stdout);
	
	return 0;
}

