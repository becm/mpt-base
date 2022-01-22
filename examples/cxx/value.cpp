/*!
 * MPT C++ library
 *   value handling in C++
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

using namespace mpt;

extern int main(int , char * const [])
{
	mtrace();
	
	double d[] = { 1.0, 2.0, 3.0, 4.0 };
	span<const double> s(d, 4);
	
	value v;
	
	v.set(s);
	std::cout << v.type_id() << ": " << v << std::endl;
	
	v.set("test");
	std::cout << v.type_id() << ": " << v << std::endl;
	
	v = 1.0;  std::cout << v.type_id() << ": " << v << std::endl;
	v = 2.0L; std::cout << v.type_id() << ": " << v << std::endl;
	v = 3.0f; std::cout << v.type_id() << ": " << v << std::endl;
	std::cout << std::endl;
	
	v = 10;  std::cout << v.type_id() << ": " << v << std::endl;
	v = 11L; std::cout << v.type_id() << ": " << v << std::endl;
	std::cout << std::endl;
	
	long l = 12;
	v.set(l); std::cout << v.type_id() << ": " << v << std::endl;
	int i;
	v.get(i);
	v.set(i + 1); std::cout << v.type_id() << ": " << v << std::endl;
	
	return !(i == l);
}
