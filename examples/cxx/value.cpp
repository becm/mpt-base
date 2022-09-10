/*!
 * MPT C++ library
 *   value handling in C++
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(types.h)
#include MPT_INCLUDE(meta.h)

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
	
	std::cout << basetype(v = s) << ": " << v << std::endl;
	
	const char *tst = "test";
	std::cout << basetype(v = tst) << ": " << v << std::endl;
	mpt::reference<mpt::metatype> r(mpt::metatype::create(v));
	const char *other = *r.instance();
	std::cout << other << std::endl;
	//std::cout << *r.instance() << std::endl;
	
	std::cout << basetype(v = 1.1 ) << ": " << v << std::endl;
	std::cout << basetype(v = 2.2L) << ": " << v << std::endl;
	std::cout << basetype(v = 3.3f) << ": " << v << std::endl;
	std::cout << std::endl;
	
	std::cout << basetype(v = 10  ) << ": " << v << std::endl;
	std::cout << basetype(v = 11L ) << ": " << v << std::endl;
	std::cout << std::endl;
	
	std::cout << basetype(v = 10u ) << ": " << v << std::endl;
	std::cout << basetype(v = 11uL) << ": " << v << std::endl;
	std::cout << std::endl;
	
	long l = 12;
	std::cout << basetype(v = l) << ": " << v << std::endl;
	short i;
	v.get(i);
	std::cout << basetype(v = i) << ": " << v << std::endl;
	std::cout << basetype(v = i - 13) << ": " << v << std::endl;
	
	return !(i == l);
}
