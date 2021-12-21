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
	s.set(span<double>(v, 3));
	
	std::cout << s.type() << std::endl;
	
	double *ptr = static_cast<double *>(s.reserve(5, *type_properties<double>::traits()));
	ptr[4] = 1.234;
	
	return 0;
}
