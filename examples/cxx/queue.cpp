/*!
 * test config/path object
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(queue.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

extern int main(int , char * const [])
{
	mpt::Pipe<char> p;
	
	mtrace();
	
	p.push('a');
	p.push('l');
	p.push('l');
	p.push('o');
	p.unshift('h');
	
	std::cout << p.data() << std::endl;
	
	return 0;
}
