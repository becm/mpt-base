/*!
 * MPT core library description
 */

#include "release.h"
#include "version.h"
#include "libinfo.h"

extern void _start(void)
{
	_library_ident("MPT Plotting Library");
	
	_library_task("layout element definitions");
	_library_task("graph data generation");
	
	_exit(0);
}
