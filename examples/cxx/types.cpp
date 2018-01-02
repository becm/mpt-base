/*!
 * test config/path object
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

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
		if (!t || !value::format::set(mpt::typeIdentifier(*t))) {
			return false;
		}
		return value::set(*this, t);
	}
	int type() const
	{
		return _fmt[0];
	}
};

extern int main(int, char *[])
{
	mtrace();
	Value v;
	
	long l = -5;
	v.set(&l);
	std::cout << "long(" << v.type() << ") = " << v << std::endl;
	
	unsigned long u = 5;
	v.set(&u);
	std::cout << "ulong(" << v.type() << ") = " << v << std::endl;
	
	float f(5);
	std::cout << "float(" << mpt::typeIdentifier(f) << ") = " << f << std::endl;
	double d(5);
	std::cout << "double(" << mpt::typeIdentifier(d) << ") = " << d << std::endl;
	mpt::float80 r = d;
	std::cout << "float80(" << mpt::typeIdentifier(r) << ") = " << r << std::endl;
	long double e = 5;
	std::cout << "long double(" << mpt::typeIdentifier(e) << ") = " << e << std::endl;
	
	mpt::Slice<double> t(&d, 1);
	std::cout << "Slice<" << mpt::typeIdentifier(d) <<">(" << mpt::typeIdentifier(t) << ") = " << t << std::endl;
	v.set(&t);
	std::cout << "value(<" << v.type() << ">) = " << v << std::endl;
}
