/*!
 * test config/path object
 */
#include <typeinfo>
#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(node.h)
#include MPT_INCLUDE(config.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

static void printCfg(int depth, const mpt::span<const mpt::config::item> list)
{
	for (auto &a : list) {
		for (int i = 0; i < depth; ++i) std::cout << '.';
		std::cout << a.name();
		mpt::metatype *mt = a.instance();
		if (mt) {
			std::cout << " = ";
			const char *content = *mt;
			if (content) std::cout << content;
		}
		std::cout << std::endl;
		printCfg(depth + 1, a.elements());
	}
}

extern int main(int argc, char * const argv[])
{
	mpt::config::root conf;
	mpt::convertable *val;
	const char *name;
	
	mtrace();
	
	if (conf.set("a.value.text", "Der täĸẞŦ")
	    && (val = conf.get("a.value.text"))) {
		name = *val;
		std::cout << typeid(*val).name() << " -> " << name << std::endl;
	}
	for (int i = 1; i < argc; ++i) {
		mpt::mpt_config_load(&conf, argv[i]);
	}
	conf.set("a*value*text", "anderer", '*');
	
	printCfg(0, conf.items());
	
	return 0;
}
