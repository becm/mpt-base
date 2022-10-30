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

extern int main(int , char * const [])
{
	mtrace();
	
	double d[] = { 1.0, 2.0, 3.0, 4.0 };
	mpt::span<const double> s(d, 4), sp;
	
	// templated metatype
	mpt::metatype *m = mpt::metatype::create(s);
	m->get(sp);
	std::cout << sp << std::endl;
	// printed via mpt::convertable
	std::cout << *m <<std::endl;
	// metatype pointer query yields self-reference
	m = *m;
	m->unref();
	
	// create (small) string container
	m = mpt::metatype::create("hallo");
	std::cout << *m << std::endl;
	mpt::metatype::basic *mb = *m;
	mb->unref();
	
	// metatype with character I/O
	m = mpt::metatype::create(0, 6);
	mpt::io::interface *b = *m;
	b->write(3, "mys");
	b->write(5, "tring");
	std::cout << *m;
	b->seek(2);
	std::cout << " -> [2,]=(" << b->peek(0) << ')' << std::endl;
	m->unref();
	
	// create metatype via value template
	mpt::typed_array<double> a;
	a.insert(0, 6.6);
	a.insert(1, 1.2345);
	m = mpt::metatype::create(a);
	m->get(sp);
	std::cout << sp << std::endl;
	mpt::metatype::value<mpt::typed_array<double> > *ma = *m;
	ma->unref();
	
	// create metatype from generic value
	a.insert(1, -78);
	mpt::value v;
	v = a;
	m = mpt::metatype::create(v);
	m->get(sp);
	std::cout << sp << std::endl;
	mpt::metatype::generic *mg = *m;
	mg->unref();
	
	return 0;
}
