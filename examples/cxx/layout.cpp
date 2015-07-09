/*!
 * test config/path object
 */

#include <iostream>

#include <mpt/node.h>
#include <mpt/parse.h>

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
	mpt::Polyline pl(2);
	
	if (argc < 2 || !lay.setInput(argv[1])) return 1;
	
	pl.truncate(-1, 3);
	mpt::Transform3 t;
	pl.transform(t);
	
	mpt::Slice<mpt::dpoint> pts = pl.values();
	std::cout << pts.len() << std::endl;
	
	lay.setInput(argv[1]);
	if (!lay.load()) return 2;
}
