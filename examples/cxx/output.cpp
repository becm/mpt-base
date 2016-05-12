/*!
 * test output object
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(message.h)
#include MPT_INCLUDE(output.h)

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
	logger *log = mpt_object_logger(out);
	
	log->error(__func__, "%s", "hallo");
	mpt_output_log(out, 0, LogWarning, "%s", "hallo");
	
	out->sync();
	
	log->unref();
	
	return 0;
}
