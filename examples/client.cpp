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

#include MPT_INCLUDE(notify.h)

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
    int init(mpt::metatype * = 0);
    int step(mpt::metatype *);
};
MyClient::MyClient()
{
    _ref = mpt::mpt_output_new();
}

int MyClient::init(mpt::metatype *)
{

    mpt::object *o;
    if ((o = cast<mpt::object>())) {
        o->set("history", "/dev/stdout");
        o->set("level", "info");
        return 1;
    }
    return 0;
}
int MyClient::step(mpt::metatype *)
{ return 0; }

void MyClient::unref()
{ delete this; }

int main(int argc, char * const argv[])
{
    mtrace();

    mpt::notify n;
    if (n.init(argc, argv) < 0) {
        perror("mpt init");
        return 1;
    }
    n.setDispatch(0);

    MyClient *c = new MyClient;
    c->init();

    c->log(__func__, mpt::logger::Message | mpt::logger::LogPretty, "%s = %i", "value", 5);

    mpt::object *o;
    if ((o = c->cast<mpt::object>())) {
        for (auto p : *o) {
            std::cout << p << std::endl;
        }
    }
    n.loop();

    c->unref();
}
