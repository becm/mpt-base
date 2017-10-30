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
	std::cout << "long(" << mpt::typeIdentifier(l) << ") = " << v << std::endl;
	
	unsigned long u = 5;
	char ufmt[] = { static_cast<char>(mpt::typeIdentifier(l)), 0 };
	v.set(ufmt, &u);
	std::cout << "ulong(" << mpt::typeIdentifier(u) << ") = " << v << std::endl;
	
	float f(5);
	std::cout << "float(" << mpt::typeIdentifier(f) << ") = " << f << std::endl;
	double d(5);
	std::cout << "double(" << mpt::typeIdentifier(d) << ") = " << d << std::endl;
	mpt::float80 r = d;
	std::cout << "float80(" << mpt::typeIdentifier(r) << ") = " << r << std::endl;
	long double e = 5;
	std::cout << "long double(" << mpt::typeIdentifier(e) << ") = " << e << std::endl;
	
	mpt::Slice<double> t(&d, 1);
	char dFmt[] = { static_cast<char>(mpt::typeIdentifier(t)), 0 };
	std::cout << "Slice<" << mpt::typeIdentifier(d) <<">(" << mpt::typeIdentifier(t) << ") = " << t << std::endl;
	v.set(dFmt, &t);
	std::cout << "value(<" << dFmt << ">) = " << v << std::endl;
}
