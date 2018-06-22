/*!
 * test config/path object
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(meta.h)
#include MPT_INCLUDE(array.h)
#include MPT_INCLUDE(convert.h)

#include <iostream>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

 struct Value : public mpt::value, public mpt::value::format
{
	template <typename T>
	bool set(const T *t)
	{
		if (!t || !value::format::set(mpt::basetype(mpt::typeinfo<T>::id()))) {
			return false;
		}
		return value::set(*this, t);
	}
	uint8_t type() const
	{
		return _fmt[0];
	}
};

template <typename T>
void print()
{
	int sid = mpt::typeinfo<mpt::span <T> >::id();
	std::cout << mpt::typeinfo<T>::id() << ' ';
	std::cout << '<' << mpt::basetype(sid) << '>' << ' ';
	std::cout << sid << std::endl;
}
template <typename T>
uint8_t type(const T &)
{
	return mpt::basetype(mpt::typeinfo<T>::id());
}

extern int main(int, char *[])
{
	static const uint8_t fmt[] = { mpt::basetype(mpt::typeinfo<mpt::array>::id()), 0 };
	mtrace();
	Value v;
	
	std::cout << fmt << std::endl;
	
	print<int>();
	print<float>();
	print<mpt::array>();
	print<mpt::metatype>();
	print<mpt::metatype *>();
	print<mpt::reference_wrapper<mpt::metatype> >();
	
	long l = -5;
	v.set(&l);
	std::cout << "long(" << v.type() << ") = " << v << std::endl;
	
	unsigned long u = 5;
	v.set(&u);
	std::cout << "ulong(" << v.type() << ") = " << v << std::endl;
	
	float f(5);
	std::cout << "float(" << type(f) << ") = " << f << std::endl;
	double d(5);
	std::cout << "double(" << type(d) << ") = " << d << std::endl;
// 	mpt::float80 r = d;
// 	std::cout << "float80(" << type(r) << ") = " << r << std::endl;
	long double e = 5;
	std::cout << "long double(" << type(e) << ") = " << e << std::endl;
	
	mpt::span<double> t(&d, 1);
	std::cout << "span<" << type(d) <<">(" << type(t) << ") = " << t << std::endl;
	v.set(&t);
	std::cout << "value(<" << v.type() << ">) = " << v << std::endl;
}
