/*!
 * test config/path object
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(message.h)

#include MPT_INCLUDE(output.h)
#include MPT_INCLUDE(layout.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;

extern int main(int , char * const [])
{
	mtrace();
	
	Map<msgdest, Reference<Cycle> > p;
	Reference<Cycle> c(new Reference<Cycle>::instance);
	
	p.set(msgdest(1,2,3), c);
	p.set(msgdest(1,4,3), c);
	
	Reference<Cycle> *cp = p.get(msgdest(1,2,3));
	
	if (!cp) {
		std::cerr << "missing destination" << std::endl;
		return 1;
	}
	
	Array<Reference<Cycle>> r = p.values();
	
	for (auto &x : r) {
		Cycle *m = x.pointer();
		std::cout << m->cycles() << std::endl;
	}
	
	r = p.values(msgdest(1,4,3));
	
	return 0;
}
