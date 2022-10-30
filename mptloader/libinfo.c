/*!
 * MPT loader library description
 */

#include "../version.h"
#include "../libinfo.h"

extern void _start(void)
{
	_library_ident("MPT Loader Library");
	
	_library_task("load factory symbol from shared library");
	_library_task("create proxy metatypes for library objects");
	
	_exit(0);
}
