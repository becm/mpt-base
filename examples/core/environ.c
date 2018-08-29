/*!
 * MPT core library
 *   demonstrate environment configuration
 */

#include "table_print.c"

#include MPT_INCLUDE(node.h)
#include MPT_INCLUDE(config.h)

int main(int argc, char *argv[])
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(config) *cfg = 0;
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_STRUCT(node) *n;
	mtrace();
	
	mpt_path_set(&p, "env", -1);
	mt = mpt_config_global(&p);
	mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeConfig)), &cfg);
	
	for (++argv; --argc; ++argv) {
		mpt_config_environ(cfg, *argv, '_', 0);
	}
	mpt_config_set(cfg, "ls.colors", 0, '.', 0);
	
	mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeNode)), &n);
	mpt_gnode_traverse(n, MPT_ENUM(TraversePreOrder) | MPT_ENUM(TraverseAll), table_print, stdout);
	
	mt->_vptr->instance.unref((void *) mt);
	return 0;
}

