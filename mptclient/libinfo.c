/*!
 * MPT client library description
 */

#include "release.h"
#include "libinfo.h"

extern void _start(void)
{
	_library_ident("MPT Solver Interface Library");
	
	_library_task("read parameter from file");
	_library_task("bind backend from library");
	_library_task("setup runtime data");
	
	_exit(0);
}

