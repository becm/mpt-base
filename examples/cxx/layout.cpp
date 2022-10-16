/*!
 * test config/path object
 */

#include <iostream>

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
	
	mpt::layout lay;
	
	if (argc < 2) {
		mpt::warning(__func__, "%s", "missing layout argument");
		return 1;
	}
	if (!lay.open(argv[1])) {
		mpt::error(__func__, "%s: %s", "unable to open layout", argv[1]);
		return 2;
	}
	if (!lay.load()) {
		mpt::error(__func__, "%s: %s", "unable to load layout", argv[1]);
		return 3;
	}
	for (const mpt::item<mpt::layout::graph> &g : lay.graphs()) {
		std::cout << g.name() << " {" << std::endl;
		const mpt::object &o = *g.instance();
		for (const mpt::property &p : o) {
			mpt::color col;
			if (p.val.get(col)) {
				std::cout << "  " << p.name << " = " << col << std::endl;
				continue;
			}
			mpt::fpoint fp;
			if (p.val.get(fp)) {
				std::cout << "  " << p.name << " = [" << fp.span() << ']' << std::endl;
				continue;
			}
			std::cout << "  " << p.name << " = " << p.val << std::endl;
		}
		std::cout << "}" << std::endl;
	}
	if (!lay.reset()) {
		mpt::error(__func__, "%s: %s", "unable to reset layout parser", argv[1]);
		return 4;
	}
}
