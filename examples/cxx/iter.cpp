/*!
 * test config/path object
 */
#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(types.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

extern int main(int , char * const [])
{
	float val;
	mtrace();
	
	const double d[] = { 1, -4, 78 };
	mpt::source<double> sd(d, 3);
	
	const int i[] = { 1, 2, 3, 4, 5 };
	mpt::source<int> si(i, 5, -2);
	
	mpt::iterator *src[] = { &sd, &si };
	
	for (mpt::iterator *it : src) {
		while (it->get(val)) {
			std::cout << val << std::endl;
			it->advance();
		}
	}
	
	return 0;
}
