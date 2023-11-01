/*
 * MPT core library
 *   demonstrate config loading
 */

#include <stdlib.h>

#include "table_print.c"

#include MPT_INCLUDE(output.h)
#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(types.h)

extern int main(int argc, char *argv[])
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(config) *cfg;
	MPT_INTERFACE(logger) *log = mpt_log_default();
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	int ret = 0, i;
	
	mpt_path_set(&p, "mpt", -1);
	mt = mpt_config_global(&p);
	MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &cfg);
	
	/* load global MPT configuration */
	i = mpt_config_load(cfg, getenv("MPT_PREFIX_ETC"), log);
	
	/* load config from local directory */
	if ((i = mpt_config_load(cfg, ".", log)) < 0) {
		return 2;
	}
	if (argc < 2) {
		cfg->_vptr->process(cfg, 0, table_print, stdout);
	}
	for (i = 1; i < argc; ++i) {
		MPT_INTERFACE(convertable) *elem;
		if (!(elem = mpt_config_get(cfg, argv[i], '.', 0))) {
			fprintf(stderr, "%s: %s\n", "missing element", argv[i]);
			++ret;
		} else {
			const char *val = mpt_convertable_data(elem, 0);
			fprintf(stdout, "%s: %s\n", argv[i], val);
		}
	}
	mpt_path_set(&p, 0, 0);
	cfg->_vptr->remove(cfg, &p);
	mt->_vptr->unref(mt);
	
	return ret;
}
