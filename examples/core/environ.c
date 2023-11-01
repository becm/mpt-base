/*!
 * MPT core library
 *   demonstrate environment configuration
 */

#include "table_print.c"

#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(types.h)

int main(int argc, char *argv[])
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(config) *cfg = 0;
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	mtrace();
	
	mpt_path_set(&p, "env", -1);
	mt = mpt_config_global(&p);
	MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &cfg);
	
	for (++argv; --argc; ++argv) {
		mpt_config_environ(cfg, *argv, '_', 0);
	}
	mpt_config_set(cfg, "ls.colors", 0, '.', 0);
	
	cfg->_vptr->process(cfg, 0, table_print, stdout);
	
	mt->_vptr->unref(mt);
	return 0;
}

