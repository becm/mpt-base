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
    int process(mpt::event * = 0) __MPT_OVERRIDE;
    
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
int MyClient::process(mpt::event *)
{
    _ref = mpt::mpt_output_remote();

    mpt::object *o;
    if ((o = proxy::cast<mpt::object>())) {
        o->set(0, "w:client.out");
        if (enc) o->set("encoding", enc);
        return 1;
    }
    return 0;
}
void MyClient::unref()
{
    if (enc) free(enc);
    delete this;
}

static int doCommand(void *, mpt::event *ev)
{
    if (!ev || !ev->msg) return 0;
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

    MyClient *c = new MyClient(argv[pos]);
    c->process();

    mpt::mpt_meta_events(&d, c);
    d.set(mpt::MessageCommand, doCommand, 0);

    c->log(__func__, mpt::logger::Debug, "%s = %i", "value", 5);

    n.loop();
}
