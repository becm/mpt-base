/*!
 * MPT C++ library
 *   metatype handling in C++
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(array.h)
#include MPT_INCLUDE(meta.h)

#include MPT_INCLUDE(io.h)

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
	span<const double> s(d, 4), sp;
	
	// templated metatype
	metatype *m = new mpt::metatype::value<span<const double> >(s);
	m->get(sp);
	std::cout << sp << std::endl;
	m->unref();
	
	// create (small) string container
	m = mpt::metatype::create("hallo");
	const char *txt = *m;
	std::cout << txt << std::endl;
	m->unref();
	
	// metatype backed by character buffer
	m = mpt::metatype::create(0, 6);
	io::interface *b = *m;
	b->write(4, "str");
	txt = *m;
	std::cout << txt << std::endl;
	m->unref();
	
	// create metatype from generic value
	mpt::typed_array<double> a;
	a.insert(0, 6.6);
	a.insert(1, 1.2345);
	mpt::value v;
	v = a;
	m = mpt::metatype::create(v);
	m->get(sp);
	std::cout << sp << std::endl;
	m->unref();
	
	return 0;
}
