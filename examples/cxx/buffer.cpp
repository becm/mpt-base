/*!
 * test buffer/queue object
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(io.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

extern int main(int argc, char * const argv[])
{
	mtrace();
	
	mpt::io::buffer buf;
	
	// write arguments (null-terminated) to buffer
	for (int i = 0; i < argc; ++i) {
		buf.write(1, argv[i], strlen(argv[i]) + 1);
	}
	// consume first argument up to path separator
	const char *v;
	if ((v = std::strrchr(*argv, '/'))) {
		buf.shift(v + 1 - *argv);
	}
	// print arguments in buffer
	while (buf.iterator::get(v)) {
		std::cout << v << std::endl;
		if (buf.advance() <= 0) {
			break;
		}
	}
	return 0;
}

