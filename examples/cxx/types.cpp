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
	
	long l = -5;
	std::cout << "long:  " << mpt::typeIdentifier(l) << " = " << mpt::value("l", &l) << std::endl;
	
	unsigned long u = 5;
	char ufmt[] = { mpt::typeIdentifier(l), 0 };
	std::cout << "ulong: " << mpt::typeIdentifier(u) << " = " << mpt::value(ufmt, &u) << std::endl;
	
	float f(5);
	std::cout << mpt::typeIdentifier(f) << " = " << f << std::endl;
	double d(5);
	std::cout << mpt::typeIdentifier(d) << " = " << d << std::endl;
	mpt::float80 r = d;
	std::cout << mpt::typeIdentifier(r) << " = " << r << std::endl;
	long double e = 5;
	std::cout << mpt::typeIdentifier(e) << " = " << e << std::endl;
	
	
}
