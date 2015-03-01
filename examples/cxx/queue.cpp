/*!
 * test config/path object
 */

#include <iostream>

#include <mpt/queue.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

extern int main(int argc, char *argv[])
{
	mpt::Pipe<char> p;
	
	mtrace();
	
	p.push('a');
	p.push('l');
	p.push('l');
	p.push('o');
	p.push('\0');
	p.unshift('h');
	
	std::cout << p.data().base() << std::endl;
	
	return 0;
}