/*!
 * interfaces to display library parameters
 */

#include <unistd.h>

#ifdef __GNUC__
# ifdef __linux__
#  ifdef __x86_64__
const char _lib_interp[] __attribute__((section(".interp"))) = "/lib64/ld-linux-x86-64.so.2";
#  elif __i386
const char _lib_interp[] __attribute__((section(".interp"))) = "/lib/ld-linux.so.2";
#  endif /* <architecture> */
# endif /* <system> */
#endif /* <compiler> */


static void _library_ident(const char *ident)
{
	int len = 0;
	
	while (ident[len]) len++;
	len = write(STDOUT_FILENO, ident, len);
	
#ifdef SHLIB_INFO
	ident = " ("SHLIB_INFO")\0";
	len = 0;
	while (ident[len]) len++;
	len = write(STDOUT_FILENO, ident, len);
#endif
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

