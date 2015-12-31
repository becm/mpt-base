/*!
 * instance of MPT client
 */

#include <mpt/client.h>
#include <mpt/output.h>

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

    o->set("history", "/dev/stdout", 0);
    o->set("level", "debug2", 0);

    o->message(__func__, mpt::client::LogLevel | mpt::LogFile, "%s = %i", "value", 5);

    c->init();
    c->unref();
}
