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

extern int main(int , char * const [])
{
	mpt::Config conf;
	mpt::metatype *m;
	const char *name;
	
	mtrace();
	
	conf.environ("*");
	
	if ((m = conf.get("desktop.session"))) {
		name = *m;
		std::cout << name << std::endl;
	}
	
	if (conf.set("hallo.ich bin.text", "Der täĸẞŦ")
	    && (m = conf.get("hallo.ich bin.text"))) {
		name = *m;
		std::cout << typeid(*m).name() << "=" << name << std::endl;
	}
	conf.set("hallo*ich bin*text", "anderer", '*');
	mpt::path p('/', 0, "hallo/ich bin/text");
	
	if ((m = conf.query(&p))) {
		name = *m;
		std::cout << " -> " << name << std::endl;
	}
	return 0;
}
