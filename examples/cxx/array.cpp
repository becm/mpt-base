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

using namespace mpt;

extern int main(int , char * const [])
{
	mtrace();
	
	Array<Reference<metatype> *> d;
	PointerArray<Reference<metatype> > p;
	Array<Reference<metatype> > a;
	
	a.insert(1, new Metatype<double>(2));
	p.insert(0, a.get(1));
	
	d = p;
	
	return 0;
}
