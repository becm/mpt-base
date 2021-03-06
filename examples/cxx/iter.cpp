/*!
 * test config/path object
 */
#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(meta.h)
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
	
	metatype *m = mpt_iterator_create("1 2 3");
	
	iterator *it = typecast<iterator>(*m);
	
	double val;
	while (it->consume<double>(val)) {
		std::cout << val << std::endl;
	}
	m->unref();
	
	return 0;
}
