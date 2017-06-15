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
    MyClient(const char *);
    virtual ~MyClient() { }
    
    void unref() __MPT_OVERRIDE;
    int init(mpt::iterator * = 0) __MPT_OVERRIDE;
    int step(mpt::iterator *) __MPT_OVERRIDE;
    
    const char *enc;
};
MyClient::MyClient(const char *e) : enc(e)
{ }

int MyClient::init(mpt::iterator *)
{
    _ref = mpt::mpt_output_remote();

    mpt::object *o;
    if ((o = cast<mpt::object>())) {
        o->set(0, "w:client.out");
        if (enc) o->set("encoding", enc);
        return 1;
    }
    return 0;
}
int MyClient::step(mpt::iterator *)
{ return 0; }

void MyClient::unref()
{ delete this; }

int main(int argc, char * const argv[])
{
    mtrace();

    mpt::notify n;
    mpt::dispatch d;

    int pos;
    if ((pos = mpt::mpt_init(&n, argc, argv)) < 0) {
        perror("mpt init");
        return 1;
    }
    n.setDispatch(&d);

    MyClient c(argv[pos]);
    c.init();

    c.log(__func__, mpt::logger::Debug, "%s = %i", "value", 5);

    const mpt::object *o;
    if ((o = c.cast<mpt::object>())) {
        for (auto p : *o) {
            std::cout << p << std::endl;
        }
    }
    n.loop();
}
