
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core.h"

extern void _mpt_abort(const char *msg, const char *fcn, const char *file, int line)
{
	static const char sep[5] = "(), \n";
	char buf[32];
	if (msg) {
		write(STDERR_FILENO, msg, strlen(msg));
		write(STDERR_FILENO, sep+3, 1);
	}
	if (fcn) {
		write(STDERR_FILENO, fcn, strlen(fcn));
		write(STDERR_FILENO, sep, 4);
	}
	if (file) {
		write(STDERR_FILENO, file, strlen(file));
	}
	if (line >= 0) {
		int len = snprintf(buf, sizeof(buf), "[%d]", line);
		if (len > 0) {
			write(STDERR_FILENO, "line: ", 6);
			write(STDERR_FILENO, buf, len);
		}
	}
	write(STDERR_FILENO, sep+1, 1);
	write(STDERR_FILENO, sep+4, 1);
	abort();
}
