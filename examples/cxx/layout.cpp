/*!
 * test config/path object
 */

#include <mpt/plot.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

extern int main(int argc, char *argv[])
{
	mtrace();
	
	mpt::Layout lay;
	
	if (argc < 2 || !lay.open(argv[1])) return 1;
	if (!lay.load()) return 2;
}
