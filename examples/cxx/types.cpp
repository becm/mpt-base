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

extern int main(int, char *[])
{
	mtrace();
	mpt::value v;
	
	long l = -5;
	v.set("l", &l);
	std::cout << "long:  " << mpt::typeIdentifier(l) << " = " << v << std::endl;
	
	unsigned long u = 5;
	char ufmt[] = { mpt::typeIdentifier(l), 0 };
	v.set(ufmt, &u);
	std::cout << "ulong: " << mpt::typeIdentifier(u) << " = " << v << std::endl;
	
	float f(5);
	std::cout << mpt::typeIdentifier(f) << " = " << f << std::endl;
	double d(5);
	std::cout << mpt::typeIdentifier(d) << " = " << d << std::endl;
	mpt::float80 r = d;
	std::cout << mpt::typeIdentifier(r) << " = " << r << std::endl;
	long double e = 5;
	std::cout << mpt::typeIdentifier(e) << " = " << e << std::endl;
	
	mpt::Slice<double> t(&d, 1);
	char dFmt[] = { static_cast<char>(mpt::typeIdentifier(t)), 0 };
	std::cout << "Slice<d>:  " << mpt::typeIdentifier(t) << " = " << t << std::endl;
	v.set(dFmt, &l);
	std::cout << "value(Slice<d>):  " << dFmt << " = " << v << std::endl;
	
	
}
