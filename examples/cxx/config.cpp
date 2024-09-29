/*!
 * test config/path object
 */
#include <typeinfo>
#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(collection.h)
#include MPT_INCLUDE(config.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

static int printCfg(void *ctx, const mpt::identifier *id, mpt::convertable *val, const mpt::collection *sub)
{
	// indicate depth with leading dots
	int depth = *static_cast<const int *>(ctx);
	for (int i = 0; i < depth; ++i) {
		std::cout << '.';
	}
	// output name and value of element
	std::cout << id->name();
	if (val) {
		std::cout << " = ";
		const char *content = val->string();
		if (content) std::cout << content;
	}
	std::cout << std::endl;
	
	// process subtree data at increased depth level
	if (sub) {
		depth++;
		sub->each(printCfg, &depth);
	}
	return 0;
}

static int printAll(void *ctx, mpt::convertable *, const mpt::collection *sub)
{
	return sub->each(printCfg, ctx);
}

extern int main(int argc, char * const argv[])
{
	mpt::config::root conf;
	mpt::convertable *val;
	
	mtrace();
	
	if (conf.set("a.value.text", "Der täĸẞŦ")
	 && conf.get("a.value.text", val)) {
		std::cout << typeid(*val).name() << " -> " << val->string() << std::endl;
	}
	for (int i = 1; i < argc; ++i) {
		mpt::mpt_config_load(&conf, argv[i]);
	}
	conf.set("a*value*text", "anderer", '*');
	
	int depth = 0;
	conf.query(0, printAll, &depth);
	
	return 0;
}
