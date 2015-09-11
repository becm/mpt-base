/*!
 * instance of MPT client
 */

#include <mpt/queue.h>
#include <mpt/array.h>
#include <mpt/event.h>
#include <mpt/output.h>
#include <mpt/message.h>
#include <mpt/stream.h>

#include <mpt/client.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

class MyClient : public mpt::client
{
public:
    MyClient() : mpt::client(mpt::mpt_output_new()) { }
    virtual ~MyClient() { }
    
    int unref();
    int step();
};

int MyClient::step()
{ return 0; }

int MyClient::unref()
{ delete this; return 0; }

int main()
{
    mtrace();
    mpt::client *c = new MyClient;
    mpt::output *o = *c;
    o->set(mpt::property("history", "/dev/stdout"), 0);
    o->set(mpt::property("level", "debug"), 0);
    o->message(0, mpt::LogDebug, "hallo");
    c->init();
    c->unref();
}
