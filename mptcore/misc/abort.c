
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core.h"

extern void _mpt_abort(const char *msg, const char *fcn, const char *file, int line)
{
	char buf[32];
	if (msg) {
		write(STDERR_FILENO, msg, strlen(msg));
		write(STDERR_FILENO, " ", 1);
	}
	write(STDERR_FILENO, "(", 1);
	if (fcn) {
		write(STDERR_FILENO, fcn, strlen(fcn));
		write(STDERR_FILENO, "(), ", 4);
	}
	if (file) {
		write(STDERR_FILENO, file, strlen(file));
	}
	write(STDERR_FILENO, buf, snprintf(buf, sizeof(buf), "[%d]\n", line));
	
	abort();
}
