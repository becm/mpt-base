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
    
    void unref();
    int step(mpt::metatype *);
};

int MyClient::step(mpt::metatype *)
{ return 0; }

void MyClient::unref()
{ delete this; }

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
