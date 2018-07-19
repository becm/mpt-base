/*!
 * instance of MPT client
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include <iostream>

#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(message.h)

#include MPT_INCLUDE(stream.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

class MyClient : public mpt::client
{
public:
    MyClient();
    virtual ~MyClient() { }
    
    void unref() __MPT_OVERRIDE;
    int process(uintptr_t , mpt::iterator *) __MPT_OVERRIDE;
    
    int conv(int , void *) const __MPT_OVERRIDE;
protected:
    mpt::reference_wrapper<mpt::metatype> _mt;
};
MyClient::MyClient()
{
    mpt::metatype *mt = mpt::mpt_output_remote();
    mpt::property pr;
    mpt::mpt_meta_info(mt, &pr);
    mpt::debug(__func__, "%s: %s", pr.name, pr.desc);
    mpt::object *o;
    if (mt && (o = mt->cast<mpt::object>())) {
        o->set(0, "w:client.out");
    }
    _mt.set_reference(mt);
}
int MyClient::process(uintptr_t , mpt::iterator *)
{
    return mpt::event::Terminate;
}
void MyClient::unref()
{
    delete this;
}
int MyClient::conv(int type, void *ptr) const
{
    metatype *mt;
    int ret;
    if ((mt = _mt.reference()) && (ret = mt->conv(type, ptr)) > 0) {
        return Type;
    }
    return client::conv(type, ptr);
}

static int do_command(void *ptr, mpt::event *ev)
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

    int pos;
    if ((pos = mpt::mpt_init(argc, argv)) < 0) {
        perror("mpt init");
        return 1;
    }
    mpt::notify n;
    if (mpt::mpt_notify_config(&n, 0) < 0) {
        return 2;
    }
    MyClient c;
    mpt::dispatch d;
    d.set_handler(mpt::msgtype::Command, do_command, &c);
    n.set_handler(&d);
    mpt::mpt_dispatch_param(&d);

    n.loop();

    mpt::log(&c, __func__, mpt::logger::Debug, "%s = %i", "value", 5);

    const mpt::metatype *mt;
    if ((mt = mpt::config::global()->cast<mpt::config>()->get("mpt.args"))) {
        mpt::iterator *it = mt->cast<mpt::iterator>();
        const char *arg;
        while (it->consume(arg)) {
            std::cerr << arg << std::endl;
        }
    }
}
