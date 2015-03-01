/*!
 * test config/path object
 */

#include <iostream>

#include <mpt/node.h>
#include <mpt/layout.h>
#include <mpt/parse.h>

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
	
	if (!lay.setInput(argv[1])) return 1;
	
	pl.truncate(-1, 3);
	mpt::Transform3 t;
	pl.transform(t);
	
	mpt::Slice<mpt::dpoint> pts = pl.values();
	std::cout << pts.len() << std::endl;
	
	lay.setInput(argv[1]);
	if (!lay.load()) return 2;
}
