/*!
 * test array type compatibility
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
	
	double v[] = { 1.0, 2.0, 3.0, 4.0 };
	
	value_store s;
	s.set(span<const double>(v + 1, 3), 2);
	
	std::cout << s.type() << std::endl;
	
	double *ptr = s.reserve<double>(8);
	
	return !(ptr[4] == v[3]);
}
