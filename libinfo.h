/*!
 * interfaces to display library parameters
 */

#include <unistd.h>

#include "version.h"

#ifdef __GNUC__
# ifdef __linux
#  ifdef __x86_64__
const char _lib_interp[] __attribute__((section(".interp"))) = "/lib64/ld-linux-x86-64.so.2";
#  else /* __x86_64__ */
const char _lib_interp[] __attribute__((section(".interp"))) = "/lib/ld-linux.so.2";
#  endif/* __x86_64__ */
# endif/* __linux */
#endif /* __GNUC__ */


static void _library_ident(const char *ident)
{
	int len = 0;
	
	while (ident[len]) len++;
	len = write(STDOUT_FILENO, ident, len);
	
#ifdef MPT_RELEASE
	ident = " (release "MPT_RELEASE")";
#elif defined(__MPT_DATE__)
	ident = " (developer build: "__MPT_DATE__")";
#else
	ident = " (developer build)";
#endif
	len = 0;
	while (ident[len]) len++;
	len = write(STDOUT_FILENO, ident, len);
	len = write(STDOUT_FILENO, "\n", 1);
}
static void _library_task(const char *task)
{
	static const char txt[] = "  - \n";
	int len = write(STDOUT_FILENO, txt, 4);
	
	len = 0;
	while (task[len]) len++;
	
	len = write(STDOUT_FILENO, task, len);
	len = write(STDOUT_FILENO, txt+4, 1);
}

