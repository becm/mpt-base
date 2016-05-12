#include <stdio.h>
#include <stdlib.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(client.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
	char *arg;
	
	mtrace();
	
	while ((arg = mpt_readline("prompt: "))) {
		puts(arg);
		free(arg);
	}
	return 0;
}
