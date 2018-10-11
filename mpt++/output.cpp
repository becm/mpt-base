/*!
 * mpt C++ library
 *   output operations
 */

#include <cstdio>
#include <cstdarg>

#include "output.h"

__MPT_NAMESPACE_BEGIN

int output::message(const char *from, int type, const char *fmt, ... )
{
	va_list va;
	va_start(va, fmt);
	int ret = mpt_output_vlog(this, from, type, fmt, va);
	va_end(va);
	return ret;
}
logfile::logfile() : file(0), state(0), mode(0), ignore(0), lsep(0)
{ }

logfile::~logfile()
{
	if (!file
	 || file == stdin
	 || file == stdout
	 || file == stderr) {
		return;
	}
	fclose(file);
}

__MPT_NAMESPACE_END
