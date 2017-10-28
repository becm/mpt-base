/*
 * MPT core library
 *   demonstrate config loading
 */

#include "table_print.c"

#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(parse.h)

extern int main(int argc, char *argv[])
{
	int i;
	
	for (i = 1; i < argc; ++i) {
		int ret;
		if ((ret = mpt_config_load(argv[i], mpt_log_default(), 0)) < 0) {
			return 1;
		}
	}
	mpt_gnode_traverse(mpt_config_node(0), MPT_ENUM(TraversePreOrder) | MPT_ENUM(TraverseAll), table_print, stdout);
	
	return 0;
}
