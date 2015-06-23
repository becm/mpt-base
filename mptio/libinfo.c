/*!
 * MPT core library description
 */

#include "release.h"
#include "version.h"
#include "libinfo.h"

extern void _start(void)
{
	_library_ident("MPT I/O Library");
	
	_library_task("connect/bind to sockets");
	_library_task("wait for input events");
	
	_exit(0);
}
