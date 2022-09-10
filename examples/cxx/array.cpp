/*!
 * test array type compatibility
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(meta.h)
#include MPT_INCLUDE(array.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;

template <typename T>
int type(const T &)
{
	return type_properties<T>::id(true);
}

extern int main(int , char * const [])
{
	mtrace();
	
	typed_array<reference<metatype> *> d;
	pointer_array<reference<metatype> > p;
	typed_array<reference<metatype> > a;
	unique_array<metatype *> u;
	
	a.insert(1, new reference<metatype::value<double> >::type());
	p.insert(0, a.get(1));
	u.insert(4, a.get(1)->instance());
	
	d = p;
	
	typed_array<double> v;
	v.insert(3, 4);
	v.set(2, 1);
	std::cout << type(v) << '>' << type(v.elements());
	std::cout << ": " << v.elements() << std::endl;
	
	return 0;
}
