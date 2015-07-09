/*!
 * test output object
 */

#include <iostream>

#include <mpt/message.h>

#include <mpt/output.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;

extern int main(int , char * const [])
{
	mtrace();
	
	output *out = mpt_output_new();
	logger *log = out->cast<logger>();
	
	log->error(__func__, "%s", "hallo");
	mpt_output_log(out, 0, LogError, "%s", "hallo");
	
	out->sync();
	
	log->unref();
	
	return 0;
}
