/*!
 * MPT plot library
 *   type-safe value_store assignments
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
	
	value_store s;
	
	// set values at specified offset
	double v[] = { 1.0, 2.0, 3.14, 4e-123 };
	double *ptr = s.set(span<const double>(v + 1, 3), 2);
	
	// content type is defined
	std::cout << static_cast<char>(s.type()) << "@[" << span<const double>(ptr - 2, s.element_count())  << ']' << std::endl;
	
	// same type area extension
	ptr = s.reserve<double>(8);
	std::cout << static_cast<char>(s.type()) << "@[" << span<const double>(ptr, s.element_count())  << ']' << std::endl;
	
	// conflicting type reserve fails
	int *iptr = s.reserve<int>(3);
	
	return !(ptr && ptr[4] == v[3] && !iptr);
}
