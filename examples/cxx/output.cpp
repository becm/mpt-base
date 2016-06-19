/*!
 * test output object
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(message.h)
#include MPT_INCLUDE(output.h)
#include MPT_INCLUDE(node.h)

#include MPT_INCLUDE(stream.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;

extern int main(int , char * const [])
{
	mtrace();
	
	node *n = mpt_node_new(0, 0);
	delete n;
	
	output *out = new Stream;
	logger *log = mpt_object_logger(out);
	
	log->message(__func__, log->Error, "%s", "hallo");
	mpt_output_log(out, 0, LogWarning, "%s", "hallo");
	
	out->sync();
	
	log->unref();
	
	return 0;
}
