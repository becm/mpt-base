/*!
 * test config/path object
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(layout.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

extern int main(int argc, char *argv[])
{
	mtrace();
	
	mpt::Layout lay;
	
	long l;
	unsigned long u;
	mpt::print("long = '%c', '%c'", mpt::typeIdentifier(l), mpt::typeIdentifier(u));
	long double ld;
	mpt::print("long double = '%c'", mpt::typeIdentifier(ld));
	
	if (argc < 2) {
		mpt::warning(__func__, "%s", "missing layout argument");
		return 1;
	}
	if (!lay.open(argv[1])) {
		mpt::error(__func__, "%s: %s", "unable to open layout", argv[1]);
		return 2;
	}
	if (!lay.load()) return 3;
}
