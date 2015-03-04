/*!
 * MPT core library description
 */

#include "release.h"
#include "libinfo.h"

extern void _start(void)
{
	_library_ident("MPT Core Library");
	
	_library_task("list/tree manipulation and processing");
	_library_task("generic type infrastructure");
	_library_task("simple text parser");
	
	_exit(0);
}
