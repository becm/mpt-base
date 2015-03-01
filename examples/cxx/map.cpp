/*!
 * test config/path object
 */

#include <iostream>

#include <mpt/layout.h>
#include <mpt/output.h>
#include <mpt/message.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;

extern int main(int argc, char *argv[])
{
	mtrace();
	
	Map<laydest, Reference<Cycle> > p;
	Reference<Cycle> c(new Cycle);
	
	p.set(laydest(1,2,3), c);
	p.set(laydest(1,4,3), c);
	
	p.get(laydest(1,2,3));
	
	Array<Reference<Cycle> > r = p.values();
	
	r = p.values(laydest(1,4,3));
	
	return 0;
}
