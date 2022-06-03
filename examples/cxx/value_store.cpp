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
	int ret = 0;
	mtrace();
	
	value_store s;
	
	// set values at specified offset
	double v[] = { 1.0, 2.0, 3.14, 4e-123 };
	double *ptr = s.set(span<const double>(v + 1, 3), 2);
	ret |= !(ptr && ptr[2] == v[3]);
	
	// content type is defined
	span<double> sd(ptr - 2, s.element_count());
	std::cout << static_cast<char>(s.type()) << "@[" << sd  << ']' << std::endl;
	
	// same type area extension
	content<double> *cd = s.reserve<double>(8);
	std::cout << static_cast<char>(s.type()) << "@[" << cd->data()  << ']' << std::endl;
	
	// conflicting type assign fails
	int i[] = { 1, 2, 3 };
	int *iptr = s.set(span<const int>(i, 3), 0);
	ret |= !(!iptr);
	
	// new type reserve replaces old data
	content<int> *ci = s.reserve<int>(3);
	int *in_set = s.set(span<const int>(i + 1, 2), 4);
	ci = s.reserve<int>();
	std::cout << static_cast<char>(s.type()) << "@[" << ci->data() << ']' << std::endl;
	ret |= !(in_set && i[1] == in_set[0]);
	
	content<uint16_t> *cq = s.reserve<uint16_t>();
	std::cout << static_cast<char>(s.type()) << "@[" << cq->data() << ']' << std::endl;
	
	return ret;
}
