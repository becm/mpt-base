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

static void printCfg(int depth, const mpt::span<const mpt::Config::Element> list)
{
	for (auto &a : list) {
		for (int i = 0; i < depth; ++i) std::cout << '.';
		std::cout << a.name();
		mpt::metatype *mt = a.pointer();
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
	mpt::Config conf;
	const mpt::metatype *m;
	const char *name;
	
	mtrace();
	
	if (conf.set("a.value.text", "Der täĸẞŦ")
	    && (m = conf.get("a.value.text"))) {
		name = *m;
		std::cout << typeid(*m).name() << " -> " << name << std::endl;
	}
	for (int i = 1; i < argc; ++i) {
		mpt::mpt_config_load(&conf, argv[i]);
	}
	conf.set("a*value*text", "anderer", '*');
	
	printCfg(0, conf.elements());
	
	return 0;
}
