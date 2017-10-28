/*!
 * MPT core library
 *   demonstrate environment configuration
 */

#include "table_print.c"

#include MPT_INCLUDE(node.h)
#include MPT_INCLUDE(config.h)

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

