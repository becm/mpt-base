/*!
 * instance of MPT client
 */

#include <mpt/queue.h>
#include <mpt/array.h>
#include <mpt/event.h>
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
    MyClient() : mpt::client(0) { }
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
//     o->set(mpt::property("", "w:/dev/stdout"));
    o->push(6, "hallo\n");
    c->init();
    c->unref();
}
