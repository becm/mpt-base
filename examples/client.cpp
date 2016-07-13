/*!
 * instance of MPT client
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include <iostream>

#include MPT_INCLUDE(output.h)
#include MPT_INCLUDE(message.h)
#include MPT_INCLUDE(object.h)

#include MPT_INCLUDE(client.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

class MyClient : public mpt::client, public mpt::proxy
{
public:
    MyClient();
    virtual ~MyClient() { }
    
    void unref();
    int step(mpt::metatype *);
};
MyClient::MyClient()
{
    mpt::metatype *m = mpt::mpt_output_new();
    mpt::output *o;
    o = m->cast<mpt::output>();
    output.setPointer(o);

    o->set("history", "/dev/stdout", 0);
    o->set("level", "info", 0);

    if (o->addref()) {
        mpt::Reference<class mpt::logger> r(mpt::mpt_output_logger(o));
        logger = std::move(r);
    }
}

int MyClient::step(mpt::metatype *)
{ return 0; }

void MyClient::unref()
{ delete this; }

int main()
{
    mtrace();
    MyClient *c = new MyClient;

    c->log(__func__, mpt::LogMessage | mpt::LogPretty, "%s = %i", "value", 5);

    const mpt::object *o = c->output.pointer();
    for (auto p : *o) {
        std::cout << p << std::endl;
    }
    c->init();
    c->unref();
}
