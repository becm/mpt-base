/*
 * MPT core library
 *   demonstrate config loading
 */

#include "table_print.c"

#include MPT_INCLUDE(output.h)
#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(parse.h)

extern int main(int argc, char *argv[])
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(config) *cfg;
	MPT_STRUCT(node) *n;
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	int i;
	
	mpt_path_set(&p, "mpt", -1);
	mt = mpt_config_global(&p);
	mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg);
	
	if ((i = mpt_config_load(cfg, 0, mpt_log_default())) < 0) {
		return 1;
	}
	if (argc < 2) {
		mt->_vptr->conv(mt, MPT_ENUM(TypeNode), &n);
		mpt_gnode_traverse(n, MPT_ENUM(TraversePreOrder) | MPT_ENUM(TraverseAll), table_print, stdout);
	}
	for (i = 1; i < argc; ++i) {
		const MPT_INTERFACE(metatype) *elem;
		if (!(elem = mpt_config_get(cfg, argv[i], '.', 0))) {
			fprintf(stderr, "%s: %s\n", "missing element", argv[i]);
		} else {
			const char *val = mpt_meta_data(elem, 0);
			fprintf(stdout, "%s: %s\n", argv[i], val);
		}
	}
	mpt_path_set(&p, 0, 0);
	cfg->_vptr->remove(cfg, &p);
	mt->_vptr->ref.unref((void *) mt);
	
	return 0;
}
