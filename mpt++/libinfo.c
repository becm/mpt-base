/*!
 * MPT C++ library description
 */

#include "version.h"
#include "libinfo.h"

extern void _start(void)
{
	_library_ident("MPT C++ Base Library");
	
	_library_task("graphic data transformation and storage");
	_library_task("generic interface class for metatypes");
	_library_task("buffer/queue/stream/socket based I/O");
	_library_task("layout item implementations");
	
	_exit(0);
}

