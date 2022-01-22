/*!
 * test output object
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(stream.h)
#include MPT_INCLUDE(io.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;

extern int main(int , char * const [])
{
	mtrace();
	
	io::stream::input *out = io::stream::input::create();
	
	out->open("/dev/stdout", "w");
	
	mpt_output_log(out, __FUNCTION__, logger::Warning, "%s", "hallo");
	
	out->sync();
	out->unref();
	
	return 0;
}
