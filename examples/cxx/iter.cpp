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
	iterator *it;
	double val;
	
	if (it &= *m) {
		while (it->get(val)) {
			std::cout << val << std::endl;
			it->advance();
		}
	}
	m->unref();
	
	double vals[] = { 1, 2, 3 };
	source<double> s(vals, 3);
	while (s.get(val)) {
		std::cout << val << std::endl;
		s.advance();
	}
	
	return 0;
}
