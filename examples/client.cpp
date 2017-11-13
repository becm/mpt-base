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

#include MPT_INCLUDE(stream.h)

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
    int process(uintptr_t , mpt::iterator *) __MPT_OVERRIDE;
    
    template<typename T>
    T *cast()
    {
        return proxy::cast<T>();
    }
protected:
    char *enc;
};
MyClient::MyClient(const char *e) : enc(0)
{
    if (e) enc = strdup(e);
}
int MyClient::process(uintptr_t , mpt::iterator *)
{
    _ref = mpt::mpt_output_remote();

    mpt::object *o;
    if ((o = proxy::cast<mpt::object>())) {
        o->set(0, "w:client.out");
        if (enc) o->set("encoding", enc);
    }
    return mpt::event::Terminate;
}
void MyClient::unref()
{
    if (enc) free(enc);
    delete this;
}

static int doCommand(void *ptr, mpt::event *ev)
{
    auto *c = static_cast<mpt::client *>(ptr);
    if (!ev) {
        return 0;
    }
    if (!ev->msg) {
        return c->dispatch(ev);
    }
    std::cout << ev->msg->length() << std::endl;
    return 0;
}
int main(int argc, char * const argv[])
{
    mtrace();

    mpt::notify n;
    mpt::dispatch d;

    int pos;
    if ((pos = mpt_init(&n, argc, argv)) < 0) {
        perror("mpt init");
        return 1;
    }
    n.setDispatch(&d);

    mpt::Stream *in = new mpt::Stream();
    in->open("/dev/stdin");
    mpt_notify_add(&n, -1, in);

    MyClient c(argv[pos]);

    d.set(mpt::MessageCommand, doCommand, &c);
    d.setDefault(mpt::MessageCommand);

    n.loop();
    
    c.log(__func__, mpt::logger::Debug, "%s = %i", "value", 5);
}
