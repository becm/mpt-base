#include <stdio.h>
#include <stdlib.h>

#include <mpt/client.h>

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
