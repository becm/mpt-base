/*!
 * test config/path object
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(values.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;

extern int main(int , char * const [])
{
	mtrace();
	
	map<laydest, reference_wrapper<Cycle> > p;
	reference_wrapper<Cycle> c(new reference_wrapper<Cycle>::instance);
	
	p.set(laydest(1,2,3), c);
	p.set(laydest(1,4,3), c);
	
	reference_wrapper<Cycle> *cp = p.get(laydest(1,2,3));
	
	if (!cp) {
		std::cerr << "missing destination" << std::endl;
		return 1;
	}
	
	typed_array<reference_wrapper<Cycle>> r = p.values();
	
	for (auto &x : r) {
		Cycle *m = x.reference();
		std::cout << m->stage_count() << std::endl;
	}
	
	r = p.values(laydest(1,4,3));
	
	return 0;
}
